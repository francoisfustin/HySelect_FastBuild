//////////////////////////////////////////////////////////////////////////
//  DataBObj.h : header file
//	Version: 1.0							Date: 10/04/08
//////////////////////////////////////////////////////////////////////////

#ifndef DATABOBJ_H__INCLUDED_
#define DATABOBJ_H__INCLUDED_

#include "global.h"			// needed for TA-CBX
#ifndef TACBX 
#include <afxtempl.h>
#include "DBPageFieldVersion.h"
#endif //TACBX
#include "EnBitmapAnchorPt.h"
#include "math.h"

#include "Utilities.h"
#include "CurvFit.h"
#include "SurfFit.h"
#include "DataBase.h"

#ifdef TACBX
#include "errno.h"
#endif

#define CSTRING_LINE_SIZE_MAX		1024

enum ProductCategory
{
	PC_Pressurisation,
	PC_BalancingAndControl,
	PC_ThermostaticControl,
	PC_ExpertSystemComponents
};

// Remark: if you change anything to this enum you have to adapt code in one location:
//           - In the 'DlgTPPageHC.cpp' file at the end of the class constructor (the 'm_arTargetTab' array).
// HYS-1935: TA-Smart Dp - 02 - Add a new category for the ribbon.
enum ProductSubCategory
{
	PSC_First,
	PSC_Undefined = PSC_First,
	PSC_PM_AirVent,
	PSC_PM_Separator,
	PSC_PM_ExpansionVessel,
	PSC_PM_TecBox,
	PSC_PM_SofteningAndDesalination,
	PSC_PM_SafetyValve,
	PSC_BC_RegulatingValve,
	PSC_BC_DpController,
	PSC_BC_BalAndCtrlValve,
	PSC_BC_PressureIndepCtrlValve,
	PSC_BC_CombinedDpCBalCtrlValve,
	PSC_BC_ControlValve,
	PSC_BC_SmartControlValve,
	PSC_BC_SmartDpC,
	PSC_BC_6WayControlValve,	// HYS-1150
	PSC_BC_HUB,
	PSC_TC_ThermostaticValve,
	PSC_TC_FloorHeatingControl,
	PSC_TC_TapWaterControl,
	PSC_ESC_ShutoffValve,
	PSC_Last = PSC_ESC_ShutoffValve
};

enum eDpStab
{
	DpStabOnBranch = 0,
	DpStabOnCV = 1,
	DpStabOnBVBypass = 2,
	DpStabNone = 4,
	DpStabLast
};

enum eMvLoc
{
	MvLocPrimary,
	MvLocSecondary,
	MvLocNone,
	MvLocLast
};

// Keep synchro with CDS_HydroMod::CvLocation
enum CvLocation
{
	CvLocNone,
	CvLocPrimSide,
	CvLocSecSide,
	CvLocLast
};

enum eDpCLoc
{
	DpCLocDownStream,
	DpCLocUpStream,
	DpCLocNone,
	DpCLocLast
};

enum ShutoffValveLoc
{
	ShutoffValveLocReturn,
	ShutoffValveLocSupply,
	ShutoffValveLocNone,
	ShutoffValveLocLast,
};

// HYS-1671: Add Smart valve in HM Calc
enum SmartValveLocalization
{
	SmartValveLocReturn,
	SmartValveLocSupply,
	SmartValveLocNone,
	SmartValveLocLast
};

enum SmartValveType
{
	SmartValveTypeUndefined,
	SmartValveTypeControl,
	SmartValveTypeDpC,
};

enum ePartOfaSet
{
	ePartOfaSetNo,
	ePartOfaSetYes,
	ePartOfaSetYesBreakOk,
	ePartOfaSetYesBreakOnMsg,
	ePartOfaSetLast
};

enum QDpCharCurveType
{
	QDpCharCurveTypeNA		= -1,
	QDpCharCurveTypeKv		= 0,
	QDpCharCurveTypeQDp		= 1
};

enum ActuatorType
{
	AT_Undefined = -1,
	AT_Electric = 0,
	AT_ThermoElectric,
	AT_ThermoStatic,
	AT_Pneumatic
};

enum RadiatorValveType
{
	RVT_Undefined = -1,
	RVT_Standard = 0,
	RVT_Presettable,
	RVT_WithFlowLimitation,
	RVT_Inserts
};

enum RadiatorInsertType
{
	RIT_Undefined = -1,
	RIT_Heimeier = 0,
	RIT_ByKv
};

enum RadiatorReturnValveMode
{
	RRVM_Undefined = -1,
	RRVM_Nothing = 0,
	RRVM_Other,
	RRVM_IMI
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DblInt
//
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_DblInt : public CData
{
	DEFINE_DATA( CDB_DblInt )

// Constructor and destructor.
protected:
	CDB_DblInt( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DblInt() {}

// Public methods.
public:
	int GetVal1() { return m_iValue1; }
	int GetVal2() { return m_iValue2; }
	void SetVal1( int iValue );
	void SetVal2( int iValue );

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int m_iValue1;
	int m_iValue2;
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_String
//
//	new:CDB_String(ID)->tableID
//	{
//		"text"
//  }
//
// Remark: IDS_ in front of a string means string ID, will be replaced by the text content of IDS from translation file
//
///////////////////////////////////////////////////////////////////////////////////

#define _STRING_LENGTH 39

class CDB_String : public CData
{
	DEFINE_DATA( CDB_String )

// Constructor and destructor.
protected:
	CDB_String( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_String() {}

// Public methods.
public:
	LPCTSTR GetString();
	void SetString( const TCHAR *ptcString );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif 
	///////////////////////////////////////////////////////////////////////////////////

	// Private variables.
private: 
	_string m_String;
	_string *m_pStr;
};		


/////////////////////////////////////////////////////////////////////////////////////////
// Compare a string with strings in a table of objects CDB_String.
// Return the extended pointer on CDB_String or _NULL_IDPTR if no corresponding string.
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef TACBX 
// pTable is the pointer on the table of CDB_String.
// str is the string to compare
const IDPTR &ScanStringTable( CTable *pTable, LPCTSTR str );

// TabIDPtr is the extended pointer on the table of CDB_String.
// str is the string to compare
const IDPTR &ScanStringTable( const IDPTR &TabIDPtr, LPCTSTR str );
#endif	// TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DblString
//
//
///////////////////////////////////////////////////////////////////////////////////

#define _DBLSTRING_LENGTHSTR1 39
#define _DBLSTRING_LENGTHSTR2 149

class CDB_DblString : public CData
{
	DEFINE_DATA(CDB_DblString)

// Attribute
TCHAR m_String1[_DBLSTRING_LENGTHSTR1 + 1];
TCHAR m_String2[_DBLSTRING_LENGTHSTR2 + 1];		

protected:
	CDB_DblString( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DblString() {}

// Public methods.
public:
	LPCTSTR GetString1() const { return m_String1; };
	LPCTSTR GetString2() const { return m_String2; };
	void SetString1( TCHAR *str ) { _tcsncpy_s( m_String1, SIZEOFINTCHAR( m_String1 ), str, _DBLSTRING_LENGTHSTR1 ); m_String1[_DBLSTRING_LENGTHSTR1] = 0; }
	void SetString2( TCHAR *str ) { _tcsncpy_s( m_String2, SIZEOFINTCHAR( m_String2 ), str, _DBLSTRING_LENGTHSTR2 ); m_String2[_DBLSTRING_LENGTHSTR2] = 0; }

	// Protected methods.
protected:
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif //TACBX
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_StringID
//
//
//	new:CDB_StringID(ID\1) ->TABLE			\1 means declared in TA-SCOPE 
//	{
//		{
//			"IDS_PN_1"						IDS_ in front of a string means string ID, will be replaced by the text content of IDS from translation file
//		}
//		600000.0							could be text, ID, enum #, or auto-increment value ++ based on the Table count during txt file reading
//		""									could be text, ID, enum #, or auto-increment value ++
//	}
///////////////////////////////////////////////////////////////////////////////////
class CDB_StringID : public CDB_String
{
	DEFINE_DATA(CDB_StringID)

protected:
	CDB_StringID( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_StringID() {}

// Public methods.
public:
	LPCTSTR GetIDstr() const { ASSERT( _T('\0') != *m_IDstr ); return m_IDstr; }
	LPCTSTR GetIDstr2() const { ASSERT ( _T('\0') != *m_IDstr2 ); return m_IDstr2; }
	
	// Return 'm_IDstr' or 'm_IDstr2' as an user type.
	template<typename T>
	bool GetIDstrAs( UINT index, T &val )
	{
		if( index > 1 ) 
		{
			ASSERT( 0 );
			return false;
		}
		bool bRet = from_string( _string( ( index > 0 ) ? GetIDstr2() : GetIDstr() ), val );
		return bRet;
	};

	// Speed optimized version of GetIDstrAs<int>()
	bool GetIDstrAsInt( UINT index, int &val )
	{
		if( index > 1 )
		{
			ASSERT( 0 );
			return false;
		}

		val = _ttoi( ( index > 0 ) ? GetIDstr2() : GetIDstr() );
		ASSERT( 0 != val || ( ERANGE != errno && EINVAL != errno ) );
		return 0 != val || ( ERANGE != errno && EINVAL != errno );
	}

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	TCHAR m_IDstr[_ID_LENGTH + 1];
	TCHAR m_IDstr2[_ID_LENGTH + 1];
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Connect
//
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_Connect : public CDB_StringID
{
	DEFINE_DATA( CDB_Connect )
		
protected:
	CDB_Connect( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Connect() {}

	// Public methods.
public:
	// Warning could be _NULL_IDPTR.
	IDPTR GetDNMinIDPtr() { if( _T('\0') != *m_DNMinIDPtr.ID ) Extend( &m_DNMinIDPtr ); return m_DNMinIDPtr; }
	IDPTR GetDNMaxIDPtr() { if(  _T('\0') != *m_DNMaxIDPtr.ID ) Extend( &m_DNMaxIDPtr ); return m_DNMaxIDPtr; }
	virtual void CrossVerifyID();

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif //TACBX
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	IDPTR m_DNMinIDPtr;
	IDPTR m_DNMaxIDPtr;
};		


/////////////////////////////////////////////////////////////////////////////////////////
// Compare a string with strings in a table of objects CDB_String.
// Return the extended pointer on CDB_String or _NULL_IDPTR if no corresponding string.
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX 
// pTable is the pointer on the table of CDB_String.
// str is the string to compare
const IDPTR &ScanStringIDTable(CTable *pTable, LPCTSTR str);

// TabIDPtr is the extended pointer on the table of CDB_String.
// str is the string to compare
const IDPTR &ScanStringIDTable(const IDPTR &TabIDPtr, LPCTSTR str);
#endif //TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_MultiString
//
// Allocate n lines of size TCHAR
///////////////////////////////////////////////////////////////////////////////////
class CDB_MultiString : public CData
{
	DEFINE_DATA( CDB_MultiString )

// Constructor and destructor.
protected:
	CDB_MultiString( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_MultiString() {}

// Public methods.
public:
	LPCTSTR GetString( int index );
	_string SearchSubString( _string SubStr, bool bRemoveSubString = true );
	bool SetString( int iKey, _string str );
	int	 GetNumofLines() { return (int)m_MapList.size(); }
	void DeleteAllLines() { m_MapList.clear(); }
	void RemoveKey( int Key );
	void AddKey( int iKey, _string strValue );

#ifndef TACBX
	// Return true when the String[Index] == "true" ... case unsensitive
	bool IsStrIndexTrue( int iIndex );
#endif

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif //TACBX
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	typedef std::map<int, _string> _StrMap; 
	_StrMap	m_MapList;

// Private variables.
private:
	_string *m_pStr;
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_MultiStringExt
//
// Exemple of usage
//## eHUBSV_Name					=1
//## eHUBSV_ValveChar_ID			=2
//## eHUBSV_TAIS_HubPartnerSupply	=3
//## eHUBSV_TAIS_HubPartnerSize		=4
//## eHUBSV_ImageID					=5
//
//
//new:CDB_MultiStringExt(HUBSV_STAD)->HUBSUPVALV_TAB 
//	{
//		#eHUBSV_Name						"Name"		
//		#eHUBSV_ValveChar_ID 				"ID on valve char"			
//		#eHUBSV_TAIS_HubPartnerSupply 		"TAIS_HUB_PARTNER_SUPPLY"	
//		#eHUBSV_TAIS_HubPartnerSize 		"TAIS_HUB_PARTNER_SIZE"		
//		#eHUBSV_ImageID 					"Image identificator"		
//	}		
///////////////////////////////////////////////////////////////////////////////////
class CDB_MultiStringExt : public CDB_MultiString
{
	DEFINE_DATA( CDB_MultiStringExt )

// Constructor and destructor.
protected:
	CDB_MultiStringExt( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_MultiStringExt() {}

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif //TACBX
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_MultiStringOrdered
//
// Allocate a Key to a CDB_MultiString
///////////////////////////////////////////////////////////////////////////////////
class CDB_MultiStringOrdered : public CDB_MultiString
{
	DEFINE_DATA( CDB_MultiStringOrdered )

// Constructor and destructor.
protected:
	CDB_MultiStringOrdered( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_MultiStringOrdered() {}

// Protected methods.
protected:
	void SetKeyOrder( int iKey ) { m_iKeyOrder = iKey; }
	virtual int GetKeyOrder() { return m_iKeyOrder; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO
	virtual bool Read( INPSTREAM inpf );
	virtual void Write( OUTSTREAM outf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int	m_iKeyOrder;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Doc
//
// 
///////////////////////////////////////////////////////////////////////////////////
class CDB_Doc : public CDB_MultiStringOrdered
{
	DEFINE_DATA( CDB_Doc )

// Constructor and destructor.
protected:
	CDB_Doc( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Doc() {}
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DefaultUnits
//
// Tool class no Read/Write function == NO VARIABLE 
///////////////////////////////////////////////////////////////////////////////////
class CDB_DefaultUnits : public CDB_MultiString
{
	DEFINE_DATA( CDB_DefaultUnits )

// Constructor and destructor.
protected:
	CDB_DefaultUnits( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DefaultUnits() {}

	// Public methods.
public:
	// Put the array's index declared in Unit.cpp(_UNIT_STRING_ID[i]).
	// The function will find the "i" string and compare it with the one in the 
	// localdb. The function return the local default unit index
	int GetLocalizedDefaultUnits( int i );
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Curve
//
//	Inherited from CData
//
//	new:CDB_Curve(ID)->tableID
//	{
//		n				// Number of functions associated to the abscissa.
//		x0 \ y0			// Points X0\Y0(0)\...\Y0(n-1) DpL \ +-% of deviation
//		x1 \ y1
//		...
//	}
///////////////////////////////////////////////////////////////////////////////////
class CDB_Curve : public CData
{
	DEFINE_DATA( CDB_Curve )

protected:
	CDB_Curve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Curve();

// Public methods.
public:
	double GetXmin();
	double GetYmin( int iFunction = 0 );
	double GetXmax();
	double GetYmax( int iFunction = 0 );
	double GetBoundedY( double dX, int iFunction = 0 );		// X is maximized(minimized) to Xmax(Xmin)
	
	// Get the number of functions associated to the abscissa.
	int GetFuncNumber( void ) { return m_pCurve->GetFuncNumber(); }

	CCurveFitter *GetpCurve() {	return m_pCurve; }

	// Setter.
	void DeleteCurve();
	CCurveFitter *CreateNewCurve( int iCurve );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	virtual void InterpretInheritedData( CString *pStr ) {}
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CCurveFitter *m_pCurve;			// Pointer on curve.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_TBCurve
//
//	Inherited from CDB_Curve
//
//	new:CDB_TBCurve(ID)->tableID
//	{
//		{
//			n				// Number of functions associated to the abscissa.
//			x0 \ y0			// Points X0\Y0(0)\...\Y0(n-1)
//			x1 \ y1
//			...
//		}
//		qNmin \ qNmax		// Min \ Max equalizing flow 
//		Pmin \ Pmax			// Min \ Max Target pressure min \ max
//		CurveName			// Curve name
//	}
//CDB_TBCurve::qNminqNmax
//CDB_TBCurve::PminPmax
//CDB_TBCurve::CurveNameIDS
///////////////////////////////////////////////////////////////////////////////////
class CDB_TBCurve : public CDB_Curve
{
	DEFINE_DATA( CDB_TBCurve )

protected:
	CDB_TBCurve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_TBCurve() {}

// Public methods.
public:
	double GetqNmin() const { return m_dqNmin; }
	void SetqNmin( double val ) { m_dqNmin = val; }
	double GetqNmax() const { return m_dqNmax; }
	void SetqNmax( double val ) { m_dqNmax = val; }
	double GetPmin() const { return m_dPmin; }
	void SetPmin( double val ) { m_dPmin = val;	}
	double GetPmax() const { return m_dPmax; }
	void SetPmax( double val ) { m_dPmax = val;	}
	LPCTSTR GetCurveName( void );
	void SetCurveName( _string strCurveName ) { m_strCurveName = strCurveName; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dqNmin;
	double m_dqNmax;
	double m_dPmin;
	double m_dPmax;
	_string m_strCurveName;
	_string *m_pStr;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Characteristic
//
// Base class of CDB_ValveCharacteristic and  CDB_DpCCharacteristic
// 
//	new:CDB_DpCCharacteristic(ID\1) -> DPC_CHAR_TAB
//	{
//		Discrete/Continuous enum					Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//		Single/Multi/Electronic-turn enum			Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//		SettingRounding								Allows to know how to round the result of a computing setting.
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_Characteristic : public CData
{
	DEFINE_DATA( CDB_Characteristic )

	CDB_Characteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Characteristic() {}

// Public enumeration defines.
public:
	enum eDISCRETE
	{
		DUndefined = -1,
		False = 0,
		True = 1,
		LastDISCRETE
	};

	enum eTURN
	{
		TUndefined = -1,
		Multi = 0,
		Single = 1,
		Electronic = 2,		// Example: TA-Smart
		LastTURN
	};

// Public methods.
public:
	// Check if characteristic is discrete.
	// Remark: now this property can be set to an undefined state. It's the case for some 'CDB_PICVCharacteristic'.
	//         This is why we return now a 'eBool3' instead of a simple boolean.
	eBool3 IsDiscrete();
	bool HasDiscreteProperty( void );

	// Check if the valve is single/multi turn.
	// Remark: now this property can be set to an undefined state. It's the case for some 'CDB_PICVCharacteristic'.
	//         This is why we return now a 'eBool3' instead of a simple boolean.
	eBool3 IsMultiTurn();
	bool HasMultiTurnProperty( void );

	// Allows to know what is the rounding value for the setting.
	double GetSettingRounding() { return m_dSettingRounding; }

	// HYS-1861: these 2 methods will be removed for the next sprint when we will change for the TA-Scope.
#ifdef TACBX
	// Allow to know how minimum decimal we can display with current characteristic.
	int GetOpeningMinDec( void );

	// Allow to know if we must clear all decimals after the mantises.
	bool GetOpeningMantisseClean( void );
#endif

	// Create a virtual function to get the opening from the 'CDB_ValveCharacteristic' or 'CDB_DpCCharacteristic'
	// that will be used in the GetOpeningString(x,x) function.
	virtual double GetOpening( double KvSP ) { return -1.0; }
	virtual double GetOpeningMin() { return -1.0; }
	virtual double GetOpeningMax() { return -1.0; }

	void SetDiscrete( eDISCRETE eDiscrete ) { m_eDISCRETE = eDiscrete; }
	void SetMultiTurn( eTURN eTurn ) { m_eTURN = eTurn; }
	void SetSettingRounding( double dSettingRounding ) { m_dSettingRounding = dSettingRounding; }

#ifndef TACBX
	CString GetSettingString( double dSetting, bool bWithUnits = false, bool *pfFullSetting = NULL );
#else
	_string GetSettingString( double dSetting, bool bWithUnits = false, bool bWithFullSetting = false, bool *pfFullSetting = NULL );
#endif

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	// Discrete set of data or continuous characteristic.
	eDISCRETE m_eDISCRETE;

	// Number of turn of the valve. One, multi or electronic turn.
	eTURN m_eTURN;

	// Allows to know how to round the result of a computing setting.
	double m_dSettingRounding;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_ValveCharacteristic
//
//  Remark: Kv is either the Kvsignal or the Kv of the valve.
//
//          KvSignal is used by the TA-Scope to correct measure, Kv is used by HySelect when computing.
//
//          Kvsignal is used to help TA-Scope to correct its measures when the measuring points are to close of the valve plug.
//          In this case, the Dp is too important in regards to the real pressure drop in the valve, and Kvs helps the TA-Scope to compute
//          the correct Dp. In this case, 
//
//          In some valves (as STAD for example), the measuring points are correctly positioned away from the valve plug. Thus the measuring Dp is correct
//          and does not need to be computed by the TAScope.
//
//          If we have here Kvsignal, it means that HySelect needs other Kv values when computing. This is why we have inherited classes in regards to 
//          the different kind of valve.
//
//	CDB_ValveCharacteristic can be used for valves possessing:
//	- a simple valve characteristic Kv=Kv(position)
// or
//	- the valve characteristic (Kv catalog) and 3 additional data series 
//    for flow correction: lambda coefficient, Re* and Kv from test.
//
//	new:CDB_ValveCharacteristic(ID\1) -> VALVE_CHAR_TAB
//	{
//		{
//			Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//			Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//			SettingRounding							Allows to know how to round the result of a computing setting.
//		}
//		Min measurable setting (for TA-Scope)
//		Min recommended setting (for selection)
//      Number of curves
//		Opening 1 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//		Opening 2 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//		...
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_ValveCharacteristic : public CDB_Characteristic
{
	DEFINE_DATA( CDB_ValveCharacteristic )

protected:
	CDB_ValveCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ValveCharacteristic();

// Public methods.
public:
	// Return true if viscosity correction is provided.
	virtual bool IsViscCorrExist() { return ( 4 == m_nNumberOfCurves ? true : false ); }
	
	// Get the minimum measurable setting.
	double GetMinMeasSetting();

	// Get the minimum recommended setting (for selection).
	double GetMinRecSetting();

	bool HasKvCurve( void ) { return ( 0 == m_nNumberOfCurves ) ? false : true; }
	
	// Return: the Kv max or -1.0 if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual double GetKvMax( int iDeltaT = 0 );
	
	// Return: the Kv min or -1.0 if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual double GetKvMin( int iDeltaT = 0 );
	
	// Get the Kv value from opening.
	// Return: the Kv or -1.0 if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual double GetKv( double dOpening, int iDeltaT = 0 );

	// Get the lambda value from opening.
	// Return: the lambda or -1.0 if error or no Kv curve is defined in this valve characteristic.
	double GetLambda( double dOpening );
	
	// Get the ReStar value from opening.
	// Return: the ReStar or -1.0 if error or no Kv curve is defined in this valve characteristic.
	double GetReStar( double dOpening );
	
	// Get the KvTst value from opening.
	// Return: the KvTst or -1.0 if error or no Kv curve is defined in this valve characteristic.
	double GetKvTst( double dOpening );
	
	// Overrides the 'CDB_Characteristic' public virtual method.
	// Get the maximum opening.
	// Return: the max opening or -1.0 if error or no Kv curve is defined in this valve characteristic.
	virtual double GetOpeningMax();

	// Overrides the 'CDB_Characteristic' public virtual method.
	// Get the minimum opening.
	// Return: the min opening or -1.0 if error or no Kv curve is defined in this valve characteristic.
	virtual double GetOpeningMin();
	
	// Get the opening from the desired Kv by pure interpolation. 
	// NO VISCOSITY CORRECTION considered.
	// Return: the opening or -1.0 if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual double GetOpening( double dKv, int iDeltaT = 0 );
	
	// Get the opening from the desired Kv, without any bound checking 
	// by pure interpolation. NO VISCOSITY CORRECTION considered.
	// Return: the opening or -1.0 if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual bool GetOpening( double dKv, double *pdOpening, int iDeltaT = 0 );
	
	// Get the Dp of the valve at full opening.
	// Return: the dp or -1.0 if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual double GetDpFullOpening( double dFlow, double dRho, double dNu, int iDeltaT = 0 );

	// Get the flow from pressure drop and opening. Applies viscosity correction if available.
	// To perform the calculation, give the density (rho) and viscosity (nu).
	// Return: 'true' if the calculation was correctly performed or 'false' if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual bool GetValveQ( double* pdFlow, double dDp, double dOpening, double dRho, double dNu, int iDeltaT = 0 );

	// Get the pressure drop from the flow and opening. Applies viscosity correction if available.
	// To perform the calculation, give the density (rho) and viscosity (nu).
	// Return: 'true' if the calculation was correctly performed or 'false' if error or no Kv curve is defined in this valve characteristic.
	// Remark: 'iDeltaT' is for the inherited class 'CDB_ThermoCharacteristic'.
	virtual bool GetValveDp( double dFlow, double *pdDp, double dOpening, double dRho, double dNu, int iDeltaT = 0 );
	
	// Get the opening from flow and pressure drop. Applies viscosity correction if available.
	// To perform the calculation, give the density (rho) and viscosity (nu).
	// Rounding cases:
	//		Continuous opening - case 1
	//		Discrete opening rounded towards closest position - case 2
	//		Discrete opening rounded towards the bigger Dp - case 3 (STAM)
	//		Discrete opening rounded towards the smaller Dp - case 4 (TRV)
	// Return true if the calculation was correctly performed or 'false' if error or no Kv curve is defined in this valve characteristic.
	bool GetValveOpening( double dFlow, double dDp, double *pdOpening, double dRho, double dNu, int iRounding, int iDeltaT = 0 );

	void SetNumberOfCurves( int iNumberOfCurves ) { m_nNumberOfCurves = iNumberOfCurves; }
	void SetMinMeasSetting( double dMinMeasSetting ) { m_dMinMeasSetting = dMinMeasSetting; }
	void SetMinRecSetting( double dMinRecSetting ) { m_dMinRecSetting = dMinRecSetting; }

	void SetKvCurve( CCurveFitter *pKvCurve );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables
protected:
	double m_dMinMeasSetting;				// Min measurable setting
	double m_dMinRecSetting;				// Min recommended setting (for selection)
	CCurveFitter *m_pKvCurve;				// Pointer on Kv-position characteristic(s)

// Private variables.
private:
	int m_nNumberOfCurves;					// Set to 4 when flow correction data are provided, 1 otherwise.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_FixOCharacteristic
//
//	CDB_FixOCharacteristic is only a fixed orifice characteristic
//
//  Remark - See remark in the 'CDB_ValveCharacteristic' for the difference between Kvsignal and Kv.
//
//	new:CDB_FixOCharacteristic(ID\1) -> FIXO_CHAR_TAB
//	{
//		Kv
//		Kv signal
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_FixOCharacteristic : public CData
{
	DEFINE_DATA( CDB_FixOCharacteristic )
		
protected:
	CDB_FixOCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FixOCharacteristic() {}

// Public methods.
public:
	double GetKv() { ASSERT( m_dKv > 0.0 ); return m_dKv; }
	double GetKvSignal() { ASSERT( m_dKvSignal > 0.0 ); return m_dKvSignal; }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dKv;			// The Kv for pressure drop calculation
	double m_dKvSignal;		// The Kv Signal
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_VenturiCharacteristic
//
//	CDB_VenturiCharacteristic is only a Venturi characteristic
//
//  Remark: See remark in the 'CDB_ValveCharacteristic' for the difference between Kvsignal and Kv.
//
//	new:CDB_VenturiCharacteristic(ID\1) -> VTRI_CHAR_TAB
//	{
//		Kv signal
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_VenturiCharacteristic : public CData
{
	DEFINE_DATA( CDB_VenturiCharacteristic )

// Constructor and destructor.
protected:
	CDB_VenturiCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_VenturiCharacteristic() {}

// Public methods.
public:
	double GetKvSignal() { ASSERT( m_KvSignal >0.0 ); return m_KvSignal; }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_KvSignal;		// The Kv Signal
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DpCCharacteristic
//
//	CDB_DpCCharacteristic is only a Dp controller characteristic
//
//	new:CDB_DpCCharacteristic(ID\1) -> DPC_CHAR_TAB
//	{
//		{
//			Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//			Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//			SettingRounding							Allows to know how to round the result of a computing setting.
//		}
//		Kvmin \ Kvm
//		Absolute/Relative PB				Can be #eDPCPBTYPE_Relative, #eDPCPBTYPE_Absolute or #eDPCPBTYPE_CurveID.
//		PB (Prop. band) or CurveID			PB if #eDPCPBTYPE_Relative/#eDPCPBTYPE_Absolute or CurveID if #eDPCPBTYPE_CurveID.
//		Dplmin \ Dplmax
//		qmin \ qmax (if not defined by Kvmin/Kvm respectively)
//		SP 1 \ opening 1
//		SP 2 \ opening 2
//		...
//	}
//
// Remark:
//
//		Kvmin = m3/h at a pressure drop of 1 bar and minimum opening corresponding to the p-band (+25%).
//		Kvnom = m3/h at a pressure drop of 1 bar and opening corresponding to the middle of the p-band (?pLnom).
//		Kvm = m3/h at a pressure drop of 1 bar and maximum opening corresponding to the p-band (-25%).
//
//      DpL ^
//          | *
//          |  **   +----------------------              ---+-
//          |    ***|******               |                 | 
//      DpL +-------|------****+****-------                 | Working range DpLnom +/- 25%
//      nom |       |          |    ******|                 |
//          |       |----------|----------+***           ---+-
//          |       |          |          |   **
//          |       |          |          |     *
//          +-------+----------+----------+--------> q
//                Kvmin      Kvnom      Kvmax
///////////////////////////////////////////////////////////////////////////////////
class CDB_DpCCharacteristic : public CDB_Characteristic
{
	DEFINE_DATA( CDB_DpCCharacteristic )

// Public enumeration defines.
public:
	enum PB_type
	{
		epbt_Relative = 0,
		epbt_Absolute = 1,
		epbt_CurveID = 2
	};

protected:
	CDB_DpCCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DpCCharacteristic();

// Public methods.
public:
	// Check if PB is given in absolute terms.
	bool IsProportionalBandAbsolute();
	CDB_DpCCharacteristic::PB_type GetProportionalBandType() const { return m_eProportionalBandType; }
	
	// Kvmin = m3/h at a pressure drop of 1 bar and minimum opening corresponding to the p-band (+25%).
	// Kvnom = m3/h at a pressure drop of 1 bar and opening corresponding to the middle of the p-band (?pLnom).
	// Kvm = m3/h at a pressure drop of 1 bar and maximum opening corresponding to the p-band (-25%).

	double GetKvmin() { ASSERT( m_dKvmin >= 0.0 ); return m_dKvmin; }
	double GetKvm() { ASSERT( m_dKvm > 0.0 ); return m_dKvm; }
	double GetProportionalBand( double dDpToStab );
	double GetDplmin() { ASSERT( m_dDplmin > 0.0 ); return m_dDplmin; }
	double GetDplmax() { ASSERT( m_dDplmax > 0.0 ); return m_dDplmax; }
	
	// Return Qmin. If Qmin = 0.0 then compute Qmin through Kvmin.
	double GetQmin( double dRho, double dDpToStap = 0.0 ); 

	// Return Qmax, if Qmax = 0.0 then compute Qmax through Kvmax.
	double GetQmax( double dRho );
	
	// Overrides the 'CDB_Characteristic' virtual method.
	// Get the maximum setting/opening.
	virtual double GetOpeningMax() { return m_pPresetChar->GetMaxFX(); }

	// Overrides the 'CDB_Characteristic' virtual method.
	// Get the minimum setting/opening.
	virtual double GetOpeningMin() { return m_pPresetChar->GetMinFX(); }
	
	// Get the opening from the desired SP (linked to Dpl).
	double GetOpening( double dSP );
	
	// Get the maximum SP.
	// SP is the Set Point of the spring to generate DpL.
	double GetSPMax() { return m_pPresetChar->GetMaxX(); }
	
	// Get the minimum SP.
	// SP is the Set Point of the spring to generate DpL.
	double GetSPMin() { return m_pPresetChar->GetMinX(); }
	
	// Give the flow (q). The minimum pressure drop of the DpC is then calculated.
	// Return 'true' if the calculation was correctly performed.
	bool DpCSolve( double dQ, double* pdDpC, double dRho, double dAccuracy = 1.0e-4 );

	IDPTR GetPBCurveIDPtr();
	CDB_Curve *GetpPBCurve();

	// Setter.
	void SetNumberOfCurves( int iNumberOfCurves ) { m_iNumberOfCurves = iNumberOfCurves; }
	void SetKvmin( double dKvmin ) { m_dKvmin = dKvmin; }
	void SetKvm( double dKvm ) { m_dKvm = dKvm; }
	void SetProportionalBand( double dProportionalBand ) { m_dProportionalBand = dProportionalBand; }
	void SetDplmin( double dDplmin ) { m_dDplmin = dDplmin; }
	void SetDplmax( double dDplmax ) { m_dDplmax = dDplmax; }
	void SetQmin( double dQmin ) { m_dQmin = dQmin; }
	void SetQmax( double dQmax ) { m_dQmax = dQmax; }
	void SetProportionalBandType( PB_type eProportionalBandType ) { m_eProportionalBandType = eProportionalBandType; }
	void SetPBCurveID( LPCTSTR PBCurveID ) { m_PBCurveIDPtr.SetID( PBCurveID); }

	void DeletePresetCharCurve();
	CCurveFitter *CreateNewPresetCharCurve( int iNumberOfCurves );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

	// Just for copy purpore.
	int GetNumberOfCurves() const { return m_iNumberOfCurves; }
	double GetQmin() const { return m_dQmin; }
	double GetQmax() const { return m_dQmax; }
	double GetProportionalBand() const { return m_dProportionalBand; }
	CCurveFitter *GetpPresetChar() const { return m_pPresetChar; }

// Protected variables.
protected:
	int m_iNumberOfCurves;				// Set to 1.
	double m_dKvmin;					// The Kvmin.
	double m_dKvm;						// The Kvm.
	double m_dProportionalBand;						// The proportional band in % of the 'set point' or in Pa.
	double m_dDplmin;					// The lower bound of the catalogue Dpl range.
	double m_dDplmax;					// The higher bound of the catalogue Dpl range.
	double m_dQmin;						// The minimum flow if not defined by Kvmin, 0.0 otherwise.
	double m_dQmax;						// The maximum flow if not defined by Kvm, 0.0 otherwise.
	PB_type m_eProportionalBandType;						// epbt_Relative = 0,	epbt_Absolute = 1,	epbt_CurveID = 2
	CCurveFitter *m_pPresetChar;		// Pointer on presetting SP-position (turns) characteristic.
	IDPTR m_PBCurveIDPtr;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_ThermoCharacteristic
//
//	'CDB_ThermoCharacteristic' is inherited from a 'CDB_ValveCharacteristic'. All methods from
//  'CDB_ValveCharacteristic' will concerns KvS without Delta T consideration.
//
//	new:CDB_ThermoCharacteristic(ID\1) -> VALVE_CHAR_TAB
//	{
//		{
//			{
//				Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//				Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//				SettingRounding							Allows to know how to round the result of a computing setting.
//			}
//			Min measurable setting (for TA-Scope)
//			Min recommended setting (for selection)
//			Min setting with TA-Slider (With HyTune)
//			Number of curves
//			Opening 1 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			Opening 2 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			...
//		}
//		opening 1 \ Kv 1K \ Kv 2K \ ...
//		opening 2 \ Kv 2k \ Kv 2K \ ...
//		...
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_ThermoCharacteristic : public CDB_ValveCharacteristic
{
	DEFINE_DATA( CDB_ThermoCharacteristic )

protected:
	CDB_ThermoCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ThermoCharacteristic();

public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Overriding CDB_ValveCharacteristic methods
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// In thermostatic valve, there is at now any viscosity correction.
	virtual bool IsViscCorrExist() { return false; }

	// Get the maximum Kv.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: Kv max or -1.0 if error or no characteristic curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual double GetKvMax( int iDeltaT = 0 );

	// Get the minimum Kv.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: Kv max or -1.0 if error or no characteristic curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual double GetKvMin( int iDeltaT = 0 );
	
	// Get the Kv value from opening.
	// Param: 'dOpening' is the opening on which we want to retrieve corresponding Kv.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: Kv or -1.0 if error or no characteristic curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual double GetKv( double dOpening, int iDeltaT = 0 );

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	// Get the opening from the desired Kv by pure interpolation.
	// Param: 'dKv' is the Kv on which we want to retrieve corresponding opening.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: the corresponding opening or -1.0 if error or no characteristic curve is defined in this valve characteristic.
	// Remark: NO VISCOSITY CORRECTION considered.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual double GetOpening( double dKv, int iDeltaT = 0 );

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	// Get the opening from the desired Kv without any bound checking by pure interpolation.
	// Param: 'dKv' is the Kv on which we want to retrieve corresponding opening.
	// Param: 'pdOpening' is where to save opening found.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: 'true' if opening is found or 'false' if there is error or no characteristic curve is defined in this valve characteristic.
	// Remark: NO VISCOSITY CORRECTION considered.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual bool GetOpening( double dKv, double *pdOpening, int iDeltaT = 0 );

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	// Get the Dp of the valve at full opening.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: the opening or -1.0 if error or no characteristic curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual double GetDpFullOpening( double dFlow, double dRho, double dNu, int iDeltaT = 0 );

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	// Get the flow from pressure drop and opening. Applies viscosity correction if available.
	// To perform the calculation, give the density (rho) and viscosity (nu).
	// Return: 'true' if the calculation was correctly performed of 'false'	if error or no characteristic 
	//         curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual bool GetValveQ( double* pdFlow, double dDp, double dOpening, double dRho, double dNu, int iDeltaT = 0 );

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	// Get the pressure drop from the flow and opening with Kv.
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: 'true' if the calculation was correctly performed of 'false'	if error or no characteristic
	//         curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	virtual bool GetValveDp( double dFlow, double *pdDp, double dOpening, double dRho, double dNu, int iDeltaT = 0 );
	
	// Get the opening from flow and pressure drop.
	// Param: 'iRounding' is the rounding cases:
	//		Case 1: continuous opening
	//		Case 2: discrete opening rounded towards closest position.
	//		Case 3: discrete opening rounded towards the bigger Dp (STAM).
	//		Case 4: discrete opening rounded towards the smaller Dp (TRV).
	// Param: 'iDeltaT' specifies the curve we want to use (1 for 1K, 2 for 2K, ...).
	// Return: 'true' if the calculation was correctly performed of 'false'	if error or no characteristic
	//         curve is defined in this valve characteristic.
	// Remark: - if 'iDeltaT' is set to 0, the 'CDB_ValveCharacteristic' base class will be called.
	//         - if no curve, then we automatically call the 'CDB_ValveCharacteristic' base class.
	bool GetValveOpening( double dFlow, double dDp, double *pdOpening, double dRho, double dNu, int iRounding, int iDeltaT = 0 );
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods only for thermostatic characteristic.
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Check if Kv curve at x DT Kelvin exist or not.
	// Param: 'iDeltaT' is the curve we want to use (1 for 1K, 2 for 2K, ...).
	bool IsKvDTExist( int iDeltaT ) { return ( ( iDeltaT <= m_pKvDTCurve->GetFuncNumber() ) ? true : false ); }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	CCurveFitter *m_pKvDTCurve;			// Pointer on Kv-position characteristic(s).
	bool *m_parfDTExist;				// Allow to validate/invalidate at least the first five curves (1K, 2K, ...) (see 'Read' and 'ReadText' methods).
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_QDpCharacteristic
//
//	CDB_QDpCharacteristic is only a Q-Dp characteristic defined either by Q-Dp pair or only Kv.
//
//	new:CDB_QDpCharacteristic(ID\1) -> SEP_CHAR_TAB
//	{
//		{
//			Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//			Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//			SettingRounding							Allows to know how to round the result of a computing setting.
//		}
//		Min flow
//      Nominal flow
//		Max flow
//		Curve type:	#eCurveType_Kv or #eCurveType_QDp
//		Kv	or	q 1 \ Dp 1
//				q 2 \ Dp 2
//				...
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_QDpCharacteristic : public CDB_Characteristic
{
	DEFINE_DATA( CDB_QDpCharacteristic )
				
protected:
	CDB_QDpCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_QDpCharacteristic();

// Public methods.
public:
	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : the method computes the flow in regards to Kv in the characteristic and 'dDp'. The method returns the flow if 
	//                                   it's in the range defined in the characteristic, otherwise the method returns -1.0. 'dRho' must be defined!!
	// Curve type 'QDpCharCurveTypeQDp': if 'dDp' is in the range defined by the curve, the method returns the flow. Otherwise the method returns -1.0.
	//                                   'dRho' is not used!!
	double GetQ( double dDp, double dRho = -1.0 );

	// The method returns the minimal flow as defined in the characteristic.
	double GetQmin( void );

	// The method returns the nominal flow as defined in the characteristic.
	double GetQnom( void );
	
	// The method returns the maximal flow as defined in the characteristic.
	double GetQmax( void );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : if Kv is defined in the characteristic and 'dQMax' is in the range defined in the characteristic, the method 
	//                                   returns the computed pressure drop. Otherwise the method returns -1.0. 'dRho' must be defined!!
	// Curve type 'QDpCharCurveTypeQDp': if 'dQMax' is in the range defined in the characteristic, the method returns the corresponding pressure drop. 
	//                                   Otherwise returns -1.0. 'dRho' is not used!!
	double GetDp( double dQ, double dRho = -1.0 );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : if Kv and the minimal flow are defined in the characteristic, the method returns the computed pressure drop.
	//                                   Otherwise the method returns -1.0. 'dRho' must be defined!!
	// Curve type 'QDpCharCurveTypeQDp': if the minimal flow is defined in the characteristic, the method returns the corresponding pressure drop. 
	//                                   Otherwise returns -1.0. 'dRho' is not used!!
	double GetDpMin( double dRho = -1.0 );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : if Kv and the nominal flow are defined in the characteristic, the method returns the computed pressure drop.
	//                                   Otherwise the method returns -1.0. 'dRho' must be defined!!
	// Curve type 'QDpCharCurveTypeQDp': if the nominal flow is defined in the characteristic, the method returns the corresponding pressure drop. 
	//                                   Otherwise returns -1.0. 'dRho' is not used!!
	double GetDpNom( double dRho = -1.0 );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : if Kv and the maximal flow are defined in the characteristic, the method returns the computed pressure drop.
	//                                   Otherwise the method returns -1.0. 'dRho' must be defined!!
	// Curve type 'QDpCharCurveTypeQDp': if the maximal flow is defined in the characteristic, the method returns the corresponding pressure drop. 
	//                                   Otherwise returns -1.0. 'dRho' is not used!!
	double GetDpMax( double dRho = -1.0 );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : the method returns Kv if defined otherwise -1.0. 'dQMax' and 'dRho' are not used!!
	// Curve type 'QDpCharCurveTypeQDp': if 'dQMax' is in the range defined in the characteristic, the method returns the computed Kv. 
	//                                  'dQMax' are 'dRho' must be defined!!
	double GetKv( double dQ = -1.0, double dRho = -1.0 );
	
	// Returns Kv computed in regards to 'Qmin' if curve type is 'QDpCharCurveTypeQDp' or Kv if curve type is 'QDpCharCurveTypeKv'.

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : the method returns Kv if defined otherwise -1.0. 'dRho' is not used!!
	// Curve type 'QDpCharCurveTypeQDp': if the minimal flow is defined in the characteristic, the method returns the computed Kv. 'dRho' must be defined!!
	double GetKvMin( double dRho = -1.0 );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : the method returns Kv if defined otherwise -1.0. 'dRho' is not used!!
	// Curve type 'QDpCharCurveTypeQDp': if the nominal flow is defined in the characteristic, the method returns the computed Kv. 'dRho' must be defined!!
	double GetKvNom( double dRho = -1.0 );

	// Curve type 'QDpCharCurveTypeNA' : the method returns always -1.0.
	// Curve type 'QDpCharCurveTypeKv' : the method returns Kv if defined otherwise -1.0. 'dRho' is not used!!
	// Curve type 'QDpCharCurveTypeQDp': if the maximal flow is defined in the characteristic, the method returns the computed Kv. 'dRho' must be defined!!
	double GetKvMax( double dRho = -1.0 );

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dQmin;						// The minimum flow.
	double m_dQnom;						// The nominal flow.
	double m_dQmax;						// The maximum flow.
	QDpCharCurveType m_eQDpCharCurveType;
	double m_dKv;
	CCurveFitter* m_pQDpChar;			// Pointer on Q-Dp characteristic.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_ControlProperties
//
// CDB_ControlProperties groups control properties characterizing control valves:
//
//  - 2 ways, 3 ways of 4 ways.
//  - no control, control only, presettable or presettable with Point Test.
//  - not characterized, linear or equal percentage.
//  - mixing, diving or any of both.
//  - Push to close or push to open.
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_ControlProperties : public CData
{
	DEFINE_DATA( CDB_ControlProperties )

// Public enumeration defines.
public:
	enum CV2W3W
	{
		CVUnknown = -1,
		CV2W = 0,
		CV3W = 1,
		CV4W = 2,
		CV6W = 3,
		LastCV2W3W
	};

	// HYS-1579: A combination of CVFunc is possible
	enum eCVFUNC
	{
		NoControl = 0,
		ControlOnly = 1,
		Presettable = 2,
		PresetPT = 4,					// PT = Point Test
		Electronic = 8,					// Ex: TA-Smart
		LastCVFUNC
	};
	
	enum eCTRLCHAR
	{
		NotCharacterized = 0,			// Ex: TBV-C / KT 512
		Linear = 1,						// Ex: TPV-C-3W / KTH512
		EqualPc = 2,					// Ex: TBV-CM / BV-CM / TBV-CMP / KTM
		LastCTRLCHAR
	};
	
	enum eMixDiv
	{
		None = 0,
		Mixing = 1,
		Dividing = 2,
		LastMixDiv
	};
	
	enum ePushOrPullToClose
	{
		Undef = -1,
		PullToClose = 0,
		PushToClose = 1,
		LastPushClose
	};
	
	enum eBCVType
	{
		ebcvtNone = 0,							// Standard CV
		ebcvtKvSeries = 1,						// TBV-CM
		ebcvtLiftLimit = 2,						// TA-FUSION-C 65-200
		ebcvtCharIndep = 3,						// TA-FUSION-C 32-50
		ebcvtElectronic = 4,					// Ex: TA-Smart
		ebcvtElectronicDpC = 5,					// Ex: TA-Smart Dp
		ebcvtLast
	};

	// Pay attention: it's not a enum encoded in the table_xxx.txt. It's to determine which type of actuator
	//                can operate on this control valve.
	enum CvCtrlType
	{
		eCvNU = -1,
		eCvProportional = 0,					// Backward compatibility
		eCvOnOff = 1,
		eCv3point = 2,
		eLastCvCtrlType
	};

// Constructor and destructor.
protected:
	CDB_ControlProperties( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ControlProperties() {}

// Public methods.
public:
	// Check if arguments match to the control properties.
	bool CtrlParamFits( CV2W3W cv2w3w, eCVFUNC cvFunc );
	bool CtrlParamFits( CV2W3W cv2w3w, eCVFUNC cvFunc, eCTRLCHAR ctrlchar );
	
	// Check if type of control (On/Off, 3 points or proportional) can work with control valve that has the current 
	// characteristic (no characteristic, linear or equal percentage).
	// Param: 'CvCtrlType' is the type of control that we want to use.
	bool CvCtrlTypeFits( CvCtrlType CvCtrlType );
	
	CvCtrlType GetCtrlType();

	CV2W3W GetCv2w3w() { return m_CV2W3W; }
	eCVFUNC	GetCvFunc() { return m_CVFunc; }
	eCTRLCHAR GetCvCtrlChar() { return m_CtrlChar; }
	eMixDiv GetCvMixDiv() { return m_MixDiv; }
	ePushOrPullToClose GetCvPushClose() { return m_PushClose; }
	eBCVType GetBCVType() { return m_BCVType; }
	static _string GetCvCtrlTypeStr( CDB_ControlProperties::CvCtrlType CtrlType, bool bStrShort = true );
	static _string GetCv2W3WStr( CDB_ControlProperties::CV2W3W Cv2w3w );
	static _string GetCvFuncStr( CDB_ControlProperties::eCVFUNC CVFUNC );

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CV2W3W m_CV2W3W;							// Number of ports of the valve
	eCVFUNC m_CVFunc;							// Control valve function: control only, presettable,...
	eCTRLCHAR m_CtrlChar;						// Control characteristic: linear, equal-percentage.
	eMixDiv m_MixDiv;							// Mixing or dividing 3-way valve
	ePushOrPullToClose m_PushClose;						// The actuator must push to close the valve
	eBCVType m_BCVType;	
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_AdditCharacteristic
//
// CDB_AdditCharacteristic can be used for storing and interpolating density and 
// kinematic viscosity of water/additive mixtures depending on two variables (the 
// % in weight of the additive in water and the temperature) on a cartesian grid. 
// An additional sophistication is that a limit function T_freez(x) can be
// specified for limiting the representation and the interpolation of rho and nu.
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_AdditCharacteristic : public CData
{
	DEFINE_DATA( CDB_AdditCharacteristic )

protected:
	CDB_AdditCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_AdditCharacteristic();

// Public methods.
public:
	// Get the freezing temperature.
	double GetTfreez( double dPc );
	
	double GetMaxT( double dPc );

	// Get the density and kinematic viscosity.
	SurfValReturn_enum GetAdditProp( double dPc, double dT, double* pdDensity, double* pdKinVisc, double* pdSpecifHeat, double *pdVaporPressure );

	// Determine if the additive is to be diluted (i.e. if the % in weight is really used).
	bool IsaDilutedAddit();

	// Return the maximum/minimum dilution.
	double GetMaxPc( void );
	double GetMinPc(void);

	bool IsDensityDefined();
	bool IsKinematicViscosityDefined();
	bool IsSpecificHeatDefined();
	bool IsVaporPressureDefined();

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	// Maximum number of points in the %- and T-directions.
	int m_iPc, m_iT;
	
	// Number of characteristic surfaces defined on the cartesian grid.
	const int m_nData;
	
	// Flag to indicate if limit functions 'y_min' and 'y_max' exist.
	bool m_bTmin;
	const bool m_bTmax;
	
	// Pointer on characteristic surface(s).
	CAdditFitter *m_padditFitter;

	// Storage of last surface interpolation.
	struct SurfPt_struct m_lastRes;
	enum SurfValReturn_enum m_lastSurfValReturn;
};		


////////////////////////////////////////////////////////////////
//
//	Close-off characteristics
//
//	new:CDB_CloseOffChar(ID) -> CLSOFF_CHAR_TAB
//	{
//		Linear/Circular (0/1)					Can be #eOPENTYPE_Linear or #eOPENTYPE_Circular.
//      LimitType								Can be #eLIMITTYPE_CloseOffDp or #eLIMITTYPE_InletPressure.
//		Force/Torque 1 \ Close-off Dp 1
//		Force/Torque 2 \ Close-off Dp 2
//		...
//	}
//
class CDB_CloseOffChar : public CData
{
	DEFINE_DATA( CDB_CloseOffChar )

// Public enumeration defines.
public:
	enum eOpenType
	{
		Linear = 0,
		Circular,
		LastValue
	};

	// Characteristic type
	//   - Close off Dp: Tell us in regards to a force applied on a valve by an actuator, what is the maximum Dp through the valve
	//                   accepted. If Dp is bigger, actuator can no more close the valve.
	//					 For example the MC55Y actuator has a maximum force of 600 N. We plug this actuator on a TA-FUSION-C 50 valve.
	//                   The close off Dp characteristic of the TA-FUSION-C 50 (COC_FUSION_C3250) tell us that with a force of 600 N the 
	//                   actuator can close the valve until a Dp through the valve of +/- 3.6 bar (360 kPa).
	//					 The maximum admissible Dp through a TA-FUSION-C 50 is 3.5bar (350 kPa). Thus the MC55Y can close the valve in any
	//                   case.
	//
	//  - InletPressure: For example the KTM 512 valves we have the characteristic Force(Torque) / MaxInletPressure pairs instead of
	//                   Force / CloseOffDp. In regards to a force applied on a valve by an actuator, we can know if actuator can close the valve
	//                   in regards to the current inlet pressure of the valve.
	enum eLimitType
	{
		CloseOffDp = 0,
		InletPressure,
		LimitTypeLastValue
	};

// Constructor and destructor.
protected:
	CDB_CloseOffChar( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_CloseOffChar() { delete m_pCloseOffCharacteristic; }

// Public methods.
public:
	eOpenType GetOpenType() { return m_OpenType; }
	eLimitType GetLimitType() { return m_LimitType; }
	
	// Get the maximum Close-off Dp.
	double GetCloseOffDpMax();
	
	// Get the minimum Close-off Dp.
	double GetCloseOffDpMin();
	
	// Get the Close-off Dp value from Force/Torque.
	double GetCloseOffDp( double dF );

	// Get the maximum inlet pressure maximum.
	double GetMaxInletPressureMax();

	// Get the maximum inlet pressure minimum.
	double GetMaxInletPressureMin();

	// Get the maximum inlet pressure value from Force/Torque.
	double GetMaxInletPressure( double dF );
	
	// Get maximum force/torque.
	double GetForceTorqueMax();
	
	// Get minimum force/torque.
	double GetForceTorqueMin();
	
	// Get Force/Torque from the desired Close-Off Dp.
	// Both methods 'GetForceTorqueFromDp' and 'GetForceTorqueFromInletPressure' are exactly the same. It is just to clarify
	// between both characteristic limitation types.
	double GetForceTorqueFromDp( double dCloseOffDp );
	
	// Get Force/Torque from the desired inlet pressure.
	double GetForceTorqueFromInletPressure( double dInletPressure );

	int GetPointsInCharacteristic( void ) { return m_pCloseOffCharacteristic->GetSize(); }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	double _GetForceTorque( double dValue );

// private variables.
private:
	eOpenType m_OpenType;
	eLimitType m_LimitType;
	// Pointer on Close_offDp=f(Force/Torque) characteristic(s)
	CCurveFitter *m_pCloseOffCharacteristic;
};


////////////////////////////////////////////////////////////////
//
//	PICV characteristics
//
//	Remark: - The Dpmin-qmax characteristic integrates Fc variation due to P-band
//          - See remark in the 'CDB_ValveCharacteristic' for the difference between Kvsignal and Kv.
//
//	new:CDB_PICVCharacteristic(ID) -> VALVE_CHAR_TAB
//	{
//		{
//			{
//				Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//				Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//				SettingRounding							Allows to know how to round the result of a computing setting.
//			}
//			Min measurable setting
//			Min recommended setting (for selection)
//			Number of curves
//			Opening 1 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			Opening 2 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			...
//		}
//		Kvd				Kvs of the Dp controller (Kvs is the Kv at full opening)
//		Fc				Stabilized Dp in the throttle
//		Qmax			Max flow of the valve
//		Number of curves: 0 = no curve; 1 = Qmax curve; 2 = Qmax and Dpmin curve; 3 = Stroke
//		Setting 1 \ Dpmin 1 \ qmax 1 \ Stroke 1
//		Setting 2 \ Dpmin 2 \ qmax 2 \ Stroke 2
//		...
//	}
//

#ifndef TACBX
class CDB_Actuator;
#endif

class CDB_PICVCharacteristic : public CDB_ValveCharacteristic
{
	DEFINE_DATA( CDB_PICVCharacteristic )

protected:
	CDB_PICVCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_PICVCharacteristic();

// Public methods.
public:
	double GetKvd() { return m_dKvd; }
	double GetFc();
	double GetQmax() { return m_dQmax; }
	bool QmaxCharGiven() { return m_bQmaxCharGiven; }
	bool DpminCharGiven() { return m_bDpminCharGiven; }
	bool StrokeCharGiven() { return m_bStrokeCharGiven; }

	void SetKvd( double dKvd ) { m_dKvd = dKvd; }
	void SetFc( double dFc ) { m_dFc = dFc; }
	void SetQmax( double dQmax ) { m_dQmax = dQmax; }
	void SetNumberOfCurves(int iNumberOfCurves) { m_nNumberOfPICVCharCurves = iNumberOfCurves; }
	void SetQmaxCharGiven( bool fQmaxCharGiven ) { m_bQmaxCharGiven = fQmaxCharGiven; }
	void SetDpminCharGiven( bool fDpminCharGiven ) { m_bDpminCharGiven = fDpminCharGiven; }
	void SetStrokeCharGiven( bool fStrokeCharGiven ) { m_bStrokeCharGiven = fStrokeCharGiven; }
	void SetQmaxDpminChar( CCurveFitter* pQmaxDpMinChar );

	// Returns the setting max or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetSettingMax();

	// Returns the setting min or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetSettingMin();

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	virtual double GetOpeningMin() { return GetSettingMin(); }

	// Overrides the "CDB_ValveCharacteristic" public virtual method.
	virtual double GetOpeningMax() { return GetSettingMax(); }

#ifndef TACBX
	// HYS-1270: Get the minimum adjustable setting with actuator (And HyTune).
	bool IsStrokeCurveDefined();
	double GetMinSettingWithActuator( CDB_Actuator *pclActuator );
#endif
	
	// Returns the max Qmax or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetQmaxMax();
	
	// Returns the min Qmax or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetQmaxMin();
	
	// Returns the max Dpmin or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetDpminMax();
	
	// Returns the min Dpmin or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetDpminMin();
	
	// Get the Qmax value from a given setting.
	// Returns the Qmax or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetQmax( double dSetting );
	
	// Get the Dpmin value from a given setting.
	// Returns the Dpmin or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetDpmin( double dSetting );
	
	// Get the setting from a given flow (Qmax).
	// Returns the setting or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetSettingFromQmax( double dQmax );
	
	// Get the setting from a given Dpmin.
	// Returns the setting or -1.0 if error or no valve characteristic defined in this valve characteristic.
	double GetSettingFromDpmin( double dDpmin );

	// Allow to retrieve what is the stroke for a specific setting.
	// If stroke curve is not defined, the return value will be -1.0.
	double GetStroke( double dSetting );

	CCurveFitter *GetCurve() { return m_pQmaxDpminChar; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dKvd;						// Kvs of the Dp controller
	double m_dFc;						// Pressure drop in the throttle
	double m_dQmax;						// Max flow of the PICV (at max setting)
	bool m_bQmaxCharGiven;				// Bool: 1 if qmax characteristic is provided / 0 if not
	bool m_bDpminCharGiven;				// Bool: 1 if Dpmin characteristic is provided / 0 if not
	bool m_bStrokeCharGiven;			// Bool: 1 if Stroke characteristic is provided / 0 if not
	CCurveFitter *m_pQmaxDpminChar;		// Pointer on qmax/Dpmin characteristic

// Private variables.
private:
	int m_nNumberOfPICVCharCurves;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_FLCVCharacteristic
//
//	CDB_FLCVCharacteristic is for the flow limited control valve.
//
//	new:CDB_FLCVCharacteristic(ID\1) -> FLCV_CHAR_TAB
//	{
//		{
//			{
//				Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//				Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//				SettingRounding							Allows to know how to round the result of a computing setting.
//			}
//			Min measurable setting
//			Min recommended setting (for selection)
//			Number of curves
//			Opening 1 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			Opening 2 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			...
//		}
//		Dpmax						Max differential pressure (60KPa).
//		DpminLF						Min differential pressure at low flow (10-100l/h).
//      QLFmin						Flow min for 'DpminLF' (10l/h).
//      QLFmax						Flow max for 'DpminLF' (100l/h).
//		DpminNF						Min differential pressure at normal flow (100-150l/h)
//      QNFmin						Flow min for 'DpminNF' (100l/h).
//      QNFmax						Flow max for 'DpminNF' (150l/h).
//		Setting 1 \ Flow 1
//		Setting 2 \ Flow 2
//		...
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_FLCVCharacteristic : public CDB_ValveCharacteristic
{
	DEFINE_DATA( CDB_FLCVCharacteristic )

// Protected constructor/destructor.
protected:
	CDB_FLCVCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FLCVCharacteristic();

// Public methods.
public:
	double GetDpmax( void ) { return m_dDpmax; }
	double GetDpminLF( void ) { return m_dDpminLF; }
	double GetQLFmin( void ) { return m_dQLFmin; }
	double GetQLFmax( void ) { return m_dQLFmax; }
	double GetDpminNF( void ) { return m_dDpminNF; }
	double GetQNFmin( void ) { return m_dQNFmin; }
	double GetQNFmax( void ) { return m_dQNFmax; }

	// Get maximum setting.
	double GetSettingMax() { return m_pSettingFlowChar->GetMaxX(); }
	
	// Get minimum setting.
	double GetSettingMin() { return m_pSettingFlowChar->GetMinX(); }

	// Overrides the "CDB_Characteristic" public virtual methods.
	virtual double GetOpeningMin() { return GetSettingMin(); }
	virtual double GetOpeningMax() { return GetSettingMax(); }
	
	// Get the flow value from a given setting.
	double GetQ( double dSetting ) { return m_pSettingFlowChar->GetValue( dSetting ); }
	
	// Get the Dpmin value from a given setting.
	double GetDpmin( double dSetting );
	
	// Get the setting from a given flow.
	double GetSettingFromQ( double dQ );

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dDpmax;					// Max differential pressure (60KPa).
	double m_dDpminLF;					// Min differential pressure at low flow (10-100l/h).
	double m_dQLFmin;					// Flow min for 'DpminLF' (10l/h).
	double m_dQLFmax;					// Flow max for 'DpminLF' (100l/h).
	double m_dDpminNF;					// Min differential pressure at normal flow (100-150l/h)
	double m_dQNFmin;					// Flow min for 'DpminNF' (100l/h).
	double m_dQNFmax;					// Flow max for 'DpminNF' (150l/h).
	CCurveFitter *m_pSettingFlowChar;	// Pointer on Setting/Flow characteristic.
};


////////////////////////////////////////////////////////////////
//
//	DpC BCV characteristics
//
//	Remark: - It's typically for valve with combined Dp-controller valve and a balancing & control valve (i.e: TA-COMPACT-DP)
//          - See remark in the 'CDB_ValveCharacteristic' for the difference between Kvsignal and Kv.
//			- Here the CDB_ValveCharacteristic part contains Kvsignal for measure in TA-Scope.
//
//
//	new:CDB_DpCBCVCharacteristic(ID) -> DPCBCV_CHAR_TAB
//	{
//		{
//			{
//				Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//				Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//				SettingRounding							Allows to know how to round the result of a computing setting.
//			}
//			Min measurable setting
//			Min recommended setting (for selection)
//			Number of curves
//			Opening 1 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			Opening 2 \ Kv catalogue [ \ Lambda \Re* \ Kv from test ]
//			...
//		}
//		Kvm						max Kv of the DP-controller in the valve (control part excluded).
//		Qmin					Min flow of the valve.
//		Qmax					Max flow of the valve.
//		Dplmin					Minimum Dp that can be stabilized.
//		FlowDeltaPkCurveID		Stabilized Dp characteristic (curve with relation between the flow and Dpk).
//								'Dpk' is the Dp stabilized from the internal point of the valve between the Dp-controller and the control part to
//                              the point after the load.
//		SettingKvcCurveID		Kvc characteristic (curve with relation between setting and Kvc,D).
//								'c' because it concerns the control part of the valve and 'D' for design.
//	}
//
class CDB_DpCBCVCharacteristic : public CDB_ValveCharacteristic
{
	DEFINE_DATA( CDB_DpCBCVCharacteristic )

protected:
	CDB_DpCBCVCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DpCBCVCharacteristic() {}

// Public methods.
public:
	// Retrieve Kvm: max Kv of the DP-controller in the valve (control part excluded).
	double GetKvm() { return m_dKvm; }

	double GetQmin() { return m_dQmin; }
	double GetQmax() { return m_dQmax; }
	double GetDplmin() { return m_dDplmin; }

	// Allow to retrieve differential pressure through the entire valve (Dp controller and control part) with 
	// a given flow and opening of the control part.
	double GetDp( double dFlow, double dSetting, double dRho );

	// Allow to retrieve differential pressure through the Dp controller part of the valve without the control part. 
	double GetDpp( double dFlow, double dRho );

	// Allow to retrieve differential pressure through the control part of the valve without the Dp controller part. 
	double GetDpc( double dFlow, double dSetting, double dRho );

	CDB_Curve *GetFlowDeltaPkCurve();

	// Retrieve the curve of Kv value of the control part of the valve in regards to the setting.
	CDB_Curve *GetSettingKvcCurve();

	void SetKvm( double dKvm ) { m_dKvm = dKvm; }
	void SetQmin( double dQmin ) { m_dQmax = dQmin; }
	void SetQmax( double dQmax ) { m_dQmax = dQmax; }
	void SetDplmin( double dDplmin ) { m_dDplmin = dDplmin; }
	void SetFlowDeltaPkCurveID( LPCTSTR strFlowDeltaPkCurveID );
	void SetSettingKvcCurveID( LPCTSTR strSettingKvcCurveID );

	// Allow to retrieve the Dpk corresponding to the flow.
	// Remark: 'Dpk' is the Dp stabilized from the internal point of the valve between the Dp-controller and the control part to
	// the point after the load.
	// Returns '-1.0' if error.
	double GetDpk( double dFlow );

	// Allow to retrieve the setting for the control part.
	// Returns '-1.0' if error.
	double GetSetting( double dDpc, double dFlow, double dRho );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dKvm;						// Kvs of the Dp controller.
	double m_dQmin;						// Min flow of the valve (at min setting).
	double m_dQmax;						// Max flow of the valve (at max setting).
	double m_dDplmin;					// Minimum Dp that can be stabilized.
	IDPTR m_FlowDeltaPkCurveIDPtr;
	IDPTR m_SettingKvcCurveIDPtr;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Thing
//
//	CData with: a user-locking feature (to enable and disable user modifications)
//				a 'deleted' flag (to mark deleted objects)
//				a 'hidden' flag (to mark objects to be shown only if an unlocking 
//									password is entered)
//				an 'available' flag (to identify objects of the database available 
//										in a given database version)
//				two integers for later additions of flags through a masking system
//					* 1st integer: bit 1: InCbi flag
//
//	new:CDB_Thing(ID)->tableID	Common CData syntax
//	{
//		0				isDeleted
//		1				isAvailable
//		0				isHidden
//		1				isFix
//		0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//		0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//      0               Sort integer [0-9999] (Allow to show a group of articles in the order defined with this variable).
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_Thing : public CData
{
	DEFINE_DATA( CDB_Thing )

// Private enumeration defines.
private:
	// Enum type (for integer in pos 5).
	enum enum_Int1Bits
	{
		ei1b_InCbi			= 0x01,		// InCbi flag is one when the CDB_Thing can be sent to CBI
		ei1b_TAHUB			= 0x02,		// Available for TA Hub
		ei1b_TAHUBStation	= 0x04		// Available for TA Hub Station
	};
	
	// Enum type (for integer in pos 6).
	enum enum_Int2Bits
	{
		ei2b_NotForIndSel	= 0x01,		// The product is not available for the individual selection.
		ei2b_NotForDirSel	= 0x02,		// The product is not available for the direct selection.
		ei2b_NotForHMCalc	= 0x04,		// The product is not available for the hydraulic network calculation.
	};

// Constructor and destructor.
protected:
	CDB_Thing( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Thing() {}

// Public methods.
public:
	// User-locking status.
	virtual bool IsAvailable( bool bForHub = false );

	bool IsFixed() { return ( m_Fixed != 0 ); }
	bool InCbi() { return IsBitSetInt1( ei1b_InCbi ); }
	bool IsForHub() { return IsBitSetInt1( ei1b_TAHUB ); }
	bool IsForHubStation() { return IsBitSetInt1( ei1b_TAHUBStation ); }
	bool IsAvailableForIndividualSelection() { return !IsBitSetInt2( ei2b_NotForIndSel ); }
	bool IsAvailableForDirectSelection() { return !IsBitSetInt2( ei2b_NotForDirSel ); }
	bool IsAvailableForHMCalc() { return !IsBitSetInt2( ei2b_NotForHMCalc ); }
	void SetInt1( int iValue ) { m_FreeInt1 = iValue; }
	void SetInt2( int iValue ) { m_FreeInt2 = iValue; }
	int GetInt1() { return m_FreeInt1; }
	int GetInt2() { return m_FreeInt2; }
	int GetSortInt() { return m_SortInt; }
	void SetSortInt( int iValue ) { m_SortInt = iValue; }
	void Fix() { m_Fixed = true; }
	void Unfix() { m_Fixed = false; }
	void SetFix( bool fFix ) { m_Fixed = fFix; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	bool IsBitSetInt1( enum_Int1Bits bit ) { return ( m_FreeInt1 & bit ) ? true : false; }
	bool IsBitSetInt2( enum_Int2Bits bit ) { return ( m_FreeInt2 & bit ) ? true : false; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void InterpretInheritedData( CString *ppStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int m_Fixed;		// 1 if the object is fixed in DB.
	int	m_FreeInt1;		// bit 0: InCbi flag, bit1: available for Hub, bit2: available for HubStation.
	int	m_FreeInt2;		// bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc
	int m_SortInt;		// Allow to show a group of articles in the order defined with this variable.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_HydroThing
//
//	CDB_Thing with maximum working pressure and maximum/minimum working temperatures
//
//	new:CDB_HydroThing(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				0				isDeleted
//				1				isAvailable
//				0				isHidden
//				1				isFix
//				0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//				0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//				0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//			}
//			Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//			PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//			Tmax in �C
//			Tmin in �C
//		}
//  }
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_RuledTable;
class CDB_HydroThing : public CDB_Thing
{
	DEFINE_DATA( CDB_HydroThing )
	
// Constructor and destructor.
protected:
	CDB_HydroThing( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_HydroThing();

public:
	double GetPmaxmin() { ASSERT( m_Pmaxmin >= 0.0 ); return m_Pmaxmin; }
	double GetPmaxmax() { ASSERT( m_Pmaxmax >= 0.0 ); return m_Pmaxmax; }
	double GetTmax() { ASSERT( m_Tmax >= -273.15 ); return m_Tmax; }
	double GetTmin() { ASSERT( m_Tmin >= -273.15 ); return m_Tmin; }

#ifndef TACBX
	const CString GetTempRange( bool bWithUnits = false );
#endif

	bool IsPNIncludedInPNRange( _string strPNID );
	bool IsPNIncludedInPNRange( const IDPTR &PNIDPtr );
	bool IsPNIncludedInPNRange( double dPress );

	// IDPTR of correspondent PN
	const IDPTR &GetPNIDPtr( bool bPmaxmax = true );
	LPCTSTR GetPNID() { return m_PNIDptr.ID; }
	
	// Correspondent PN name
	_string GetPN();
	
	// Fill a list with all available PN
	bool GetPNList( CRankEx *pList );

	// Correspondent PN pressure
	double GetPNPress( bool bPmaxmax = true );
	
	void SetPmaxmin( double pmax ) { m_Pmaxmin = pmax; }
	void SetPmaxmax( double pmax ) { m_Pmaxmax = pmax; }
	void SetTmax( double tmax ) { m_Tmax = tmax; }
	void SetTmin( double tmin ) { m_Tmin = tmin; }
	void SetDispPmaxmin( bool b ) { m_bDispPmaxmin = b; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	CDB_RuledTable *GetpPNRuledTab();
	void SetpPNRuledTab( CDB_RuledTable *pRT );
	void SetpPNRuledTabID( _string ID, bool bExtend );
	bool GetDispPmaxmin() { return m_bDispPmaxmin; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Member variables.
protected:
	//Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string;
	//a 'd' in front of Pmaxmin enforces it in PN string)
	double m_Pmaxmin;		// Lowest max.working static pressure
	double m_Pmaxmax;		// Highest max. working static pressure
	double m_Tmax;			// Max. working static temperature
	double m_Tmin;			// Min. working static temperature
	bool   m_bDispPmaxmin;	// If true enforce displaying Pmaxmin	
	IDPTR *m_pIDPtrPNRuledTab; // IDPTR * because most of product don't use this, escape to memory allocation for nothing
	CTable *m_pPNTab;
	IDPTR m_PNIDptr;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Pipe
//
//	Information on a specific pipe
//
//	new:CDB_Pipe(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				0			isDeleted
//				1			isAvailable
//				0			isHidden
//				1			isFix
//				0			Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//				0			Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//				0           Sort integer (Allow to show a group of articles in the order defined with this variable).
//			}
//			Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//			PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//			Tmax in �C
//			Tmin in �C
//		}
//		"Name"				Name (of the size) of the pipe
//		SizeID				Identifier of the corresponding valve size
//		Di					Internal diameter in m
//		Roughness			Roughness in m
//	}
//
///////////////////////////////////////////////////////////////////////////////////

#define _PIPE_NAME_LENGTH 31
#define _PIPE_TMIN_INIT	-50.0

class CDB_Pipe : public CDB_HydroThing
{
	DEFINE_DATA( CDB_Pipe )

// Constructor and destructor.
protected:
	CDB_Pipe( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Pipe() {}

// Public methods.
public:
	LPCTSTR GetName();
	LPCTSTR GetNameIDS() { return m_Name.c_str(); }
	double GetIntDiameter() { ASSERT( m_dInternalDiameter > 0.0 ); return m_dInternalDiameter; }
	double GetRoughness() { ASSERT( m_dRoughness > 0.0 ); return m_dRoughness; }
	double GetLinearDp( double dFlow, double dRho, double dNu );
	double GetVelocity( double dFlow );
	
	// ID of the corresponding valve size.
	LPCTSTR GetSizeID() { return m_tcSizeID; }
	
	// IDPtr of the corresponding valve size.
	const IDPTR &GetSizeIDPtr( CDataBase *pSizeDataBase = NULL );

	// Corresponding valve size.
	LPCTSTR GetSize( CDataBase *pSizeDataBase = NULL );

	// Corresponding valve size ordering key.
	int GetSizeKey( CDataBase *pSizeDataBase = NULL );

	double GetFluidVolumeByMeter() { return ( ( GetIntDiameter() * GetIntDiameter() ) / 4 * M_PI );	}

	void SetName( LPCTSTR tcName ) { m_Name = tcName; }
	void SetIntDiameter( double dInternalDiameter ) { m_dInternalDiameter = dInternalDiameter; }
	void SetRoughness( double dRoughness ) { m_dRoughness = dRoughness; }
	void SetSizeID( LPCTSTR ID ) { _tcsncpy_s( m_tcSizeID, SIZEOFINTCHAR( m_tcSizeID ), ID, SIZEOFINTCHAR( m_tcSizeID ) - 1 ); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	TCHAR m_tcSizeID[_ID_LENGTH + 1];		// Corresp. valve size identifier
	double m_dInternalDiameter;				// Internal diameter
	double m_dRoughness;						// Roughness

// Private variables.
private:
	_string m_Name;							// Pipe name
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_TAPSortKey
// used to create/compare TA Product sorting key
///////////////////////////////////////////////////////////////////////////////////
class CDB_Product;
class CProductSortKey
{
// Public enumeration & class defines.
public:
	enum PSortKeyMask
	{
		PSKM_Inverse	= 0x010,
		PSKM_All		= 0x00F,
		PSKM_Type		= 0x008,
		PSKM_PriorL		= 0x004,
		PSKM_PN			= 0x002,
		PSKM_Family		= 0x001
	};

	enum PKeyOrder
	{
		PKO_Inverse = PSKM_Inverse,
		PKO_Last = 4,
		PKO_Type = PKO_Last - 1,
		PKO_Priority = PKO_Type - 1,
		PKO_Pn = PKO_Priority - 1,
		PKO_Fam = PKO_Pn - 1
	};

	typedef struct _KeyDef
	{
		bool m_fHiOrLow;			// 'true' is the key must be saved in the high unsigned long part of the structure sLL.
		short m_nBits;				// number of bits occupied by the key.
		short m_nBitToShift;
		short m_nMask;
	}KeyDef;

	class CPSKMask
	{
	public:
		CPSKMask() { m_uiMask = (UINT)PSKM_All; }
		CPSKMask( UINT uiMask ) { m_uiMask = uiMask; }
		CPSKMask( PSortKeyMask eMask ) { m_uiMask = (UINT)eMask; }
		UINT GetMask( void ) { return m_uiMask; }
		void SetMask( UINT uiMask ) { m_uiMask = uiMask; }
		virtual UINT GetMaskAll( void ) { return (UINT)PSKM_All; }
		virtual UINT GetMaskInverse( void ) { return (UINT)PSKM_Inverse; }

	private:
		UINT m_uiMask;
	};

	struct sLL
	{
		unsigned long L;
		unsigned long H;
	};

	// Common to all.
	CProductSortKey();
	CProductSortKey( CDB_Product *pclProduct );
	CProductSortKey( sLL sLLKey );
	CProductSortKey( CDB_StringID *psidType, CDB_StringID *psidFam, CDB_StringID *psidPN, UINT uiPriority );
	virtual ~CProductSortKey() {};

	void Init( CDB_StringID *psidType, CDB_StringID* psidFam, CDB_StringID* psidPN, UINT uiPriority );
	void InitWithDouble( double dKey );

	virtual void PrepareKeyOrderMap( void );
	virtual void ComputeSortingKey( void );

	// Returns sorting key as double.
	double GetSortingKeyAsDouble( CPSKMask *pclMask );

	// Reorganize Sorting Key fields to increase (or diminish) their relative weight.  
	// Ellipse function that take an undefined number of parameters.
	// First ellipse parameter specify the info to have the highest priority.
	// Each parameter that follow will have a decreased priority.
	// Parameters that aren't specified will be placed after with a lower
	// priority following the guide lines of the GetSortingKeyAsDouble() function.
	// The last parameter must be -1 to tell it was the last one.
	double GetSortingKeyCustomAsDouble( CPSKMask *pclMask, ... );

	LONGLONG GetPartialSortingKey( int &iNbreTotBits, CPSKMask *pclMask, ... );

	int GetNbrBitsKeyUsed( int iKey );

	// Returns sorting key as struct 'sLL'.
	sLL GetSortingKeyAsLL( CPSKMask *pclMask );

	// Return 0 if matching, -1 if this > pSK, 1 if this < pSK
	int Compare( CProductSortKey *pSK, CPSKMask *pclMask );
	
	bool operator==( CProductSortKey *pSK );
	
// Protected variables.
protected:
	union uSortingKey
	{
		sLL LL;
		unsigned char ar[sizeof( sLL )];
	}m_usKey;
	
	KeyDef m_arProductSortKey[4];
	const KeyDef *m_pKeyOrderList;
	int m_iKeyOrderSize;

	CDB_StringID *m_psidType;
	CDB_StringID *m_psidFam;
	CDB_StringID *m_psidPN;
	double m_dPriority;

// Private methods.
private:
	void _InitProductSortKey( void );
};


class CDB_TAProduct;
class CTAPSortKey : public CProductSortKey
{
public:
	enum TAPSortKeyMask
	{
		TAPSKM_Inverse	= 0x100,
		TAPSKM_All		= 0x0FF,
		TAPSKM_Type		= 0x080,
		TAPSKM_Size		= 0x040,
		TAPSKM_PriorL	= 0x020,
		TAPSKM_Bdy		= 0x010,
		TAPSKM_Connect	= 0x008,
		TAPSKM_Version	= 0x004,
		TAPSKM_PN		= 0x002,
		TAPSKM_Fam		= 0x001
	};

	enum TAPKeyOrder
	{
		TAPKO_Inverse = TAPSKM_Inverse,
		TAPKO_Last = 8,
		TAPKO_Type = TAPKO_Last - 1,
		TAPKO_Size = TAPKO_Type - 1,
		TAPKO_Priority = TAPKO_Size - 1,
		TAPKO_BdyMat = TAPKO_Priority - 1,
		TAPKO_Conn = TAPKO_BdyMat - 1,
		TAPKO_Vers = TAPKO_Conn - 1,
		TAPKO_Pn = TAPKO_Vers - 1,
		TAPKO_Fam = TAPKO_Pn - 1
	};

	class CTAPSKMask : public CProductSortKey::CPSKMask
	{
	public:
		CTAPSKMask() : CPSKMask( (UINT)TAPSKM_All ) {}
		CTAPSKMask( UINT uiMask ) : CPSKMask( uiMask ) {}
		CTAPSKMask( TAPSortKeyMask eMask ) { SetMask( (UINT)eMask ); }

		// Overrides 'CProductSortKey::CSKMask' public methods.
		virtual UINT GetMaskAll( void ) { return (UINT)TAPSKM_All; }
		virtual UINT GetMaskInverse( void ) { return (UINT)TAPSKM_Inverse; }
	};
	
public:
	CTAPSortKey();
	CTAPSortKey( CDB_TAProduct *pTAP );
	CTAPSortKey( sLL sLLKey );
	CTAPSortKey( CDB_StringID *psidType, CDB_StringID *psidSize, CDB_StringID *psidFam, CDB_StringID *psidBdy, CDB_StringID *psidConn,
				CDB_StringID *psidVers, CDB_StringID *psidPN, UINT uiPriority );
	
	virtual ~CTAPSortKey() {}

	void Init( CDB_StringID *psidType, CDB_StringID *psidSize, CDB_StringID *psidFam, CDB_StringID *psidBdy, CDB_StringID *psidConn,
				CDB_StringID *psidVers, CDB_StringID *psidPN, UINT uiPriority );

	// Overrides the 'CProductSortKey' public virtual methods.
	virtual void PrepareKeyOrderMap( void );
	virtual void ComputeSortingKey( void );
	
// Protected variables.
protected:
	CDB_StringID *m_psidSize;
	CDB_StringID *m_psidBdy;
	CDB_StringID *m_psidConn;
	CDB_StringID *m_psidVers;
	KeyDef m_arTAPProductSortKey[8];

// Private methods.
private:
	void _InitTAProductSortKey( void );
};


#define _PRODUCT_REF_LENGTH 23
#define _PRODUCT_NAME_LENGTH 50
#define _PRODUCT_DESCRIPTION_LENGTH 256

#define MAKEWPARAMDISPLAYDOC( a, b )	MAKEWPARAM( (WORD)a, (WORD)b )
#define GETDOCSHEETTYPE( a )			LOWORD( a )
#define GETDOCSHEETPRODUCT( a )			HIWORD( a )


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Product
//
//	new:CDB_Product(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				0				isDeleted
//				1				isAvailable
//				0				isHidden
//				1				isFix
//				0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//				0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//				0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//			}
//			Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//			PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//			Tmax in �C
//			Tmin in �C
//		}
//		"Reference"				Reference to the product.
//		"Name"					Name of the product
//		TypeID					Identifier of the valve type 
//		FamilyID				Identifier of the family of the product
//		VersionID				Identifier of the version of the product
//		PriorityLevel			Level of priority for selection [1,10]
//		Comment					Product comment
//		ProdImgID				Product image ID
//		AccessGroupID			Identifier of the accessory group
//		PartOfaSet				Boolean indicating if the valve is part of a ActSet
//		TableSetID				ID of the table containing the set
//		Price					Price of the product
//		ProductUseFlag			#eProduct, #eAccessory, #eBoth Integer to indicate if the product is only a product, only accessory or both
//		AccessorySeriesID		Identifier of the series of the accessory (mainly used in DlgDirSel)
//		isAttached				0/1: 1 if accessory cannot be ordered separately (for DlgDirSel)
//		AccTypeID				Identifier of the accessory type if #eBoth 
//		AccFamilyID				Identifier of the accessory family if #eBoth
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CTableSet;
class CDB_Product : public CDB_HydroThing
{
	DEFINE_DATA( CDB_Product )

protected:
	CDB_Product(CDataBase *pDataBase, LPCTSTR ID);
	virtual ~CDB_Product() {}

// Public enumeration defines.
public:

	enum eDOCSHEETTYPE
	{
		Qrs,			// Quick reference sheet
		Cat,			// Catalog sheet
		Last
	};

	// Installation layout:	#eWallHanged, #VesselTop, #eFloorStanding, #eFloorStandingNextTo, #eFloorStandingInFront, #eVesselTop or #eIntegrated
	// BE CAREFULL this flag is combined with compatibility flag that use 1st byte
	// These variables are common to CDB_Vessel and CDB_TecBox. This is why there are declared here.
	enum eInstalLayout
	{
		eiUndefined = 0,
		eilFirst = 0x01,
		eilWallHanged = eilFirst,
		eilVesselTop = 0x02,
		eilFloorStanding = 0x04,
		eilFloorStandingNextTo = 0x08,
		eilFloorStandingInFront = 0x10,
		eilIntegrated = 0x20,
		eilLast = eilIntegrated
	};

	// HYS-1018
	enum eProdUse
	{
		eProduct = 0,
		eAccessory,
		eBoth
	};

// Public methods.
public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	LPCTSTR GetBodyArtNum();

	virtual LPCTSTR GetName();
	virtual void GetNameEx(_string *pStr, bool fWithUnit = true) { *pStr = GetName(); }

	// Family IDPTR.
	const IDPTR& GetFamilyIDPtr();

	// Family name.
	LPCTSTR GetFamily();

	// Family ID.
	LPCTSTR GetFamilyID() { return m_FamilyIDPtr.ID; }

	// Version IDPTR.
	const IDPTR& GetVersionIDPtr();

	// Version name.
	LPCTSTR GetVersion();
	
	// Version ID.
	LPCTSTR GetVersionID() { return m_VersionIDPtr.ID; }

	int GetPriorityLevel() { return m_iPriorityLevel; }

	// Type IDPTR.
	const IDPTR &GetTypeIDPtr();

	// Type name.
	LPCTSTR GetType();
	
	// Type ID.
	LPCTSTR GetTypeID() { return m_TypeIDPtr.ID; }

	// Return comment associated to the product.
	LPCTSTR GetComment();

	// Product image ID.
	LPCTSTR GetProdPicID() { return m_strProdPicID.c_str(); }
	
	// Retrieve ID on accessories group.
	virtual IDPTR GetAccessoriesGroupIDPtr(void);

	ePartOfaSet GetPartOfaSet() { return m_ePartOfaSet; }

	// Retrieve the table ID where are sets if exist for the current product.
	LPCTSTR GetTableSetID() { return m_strTableSetID.c_str(); }

	// Retrieve the table where are sets if exist for the current product.
	virtual CTableSet *GetTableSet();

	virtual double GetPrice() { return m_dPrice; }

	// It's an helper for the 'Copy' method.
	CProductSortKey::sLL GetSortingKeyAsLL( void ) { return m_SortingKey; }

	// HYS-1018: for accessory
	IDPTR GetSeriesIDPtr();
	int GetProductUseFlag() { return m_iProductUseFlag; }
	LPCTSTR GetAccessTypeID() { return m_AccessTypeIDPtr.ID; }
	const IDPTR &GetAccessTypeIDPtr();
	LPCTSTR GetAccessFamilyID() { return m_AccessFamilyIDPtr.ID; }
	const IDPTR &GetAccessFamilyIDPtr();
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.

	// Set the Article number & cancels useless white spaces.
	LPCTSTR SetArtNum( LPCTSTR tcArtNum );

	// Set the Name & trims undesirable white spaces.
	LPCTSTR SetName( LPCTSTR tcName );

	// Set the FamilyID & cancels undesirable white spaces.
	LPCTSTR SetFamilyID( LPCTSTR tcFamilyID );

	void SetTypeID( LPCTSTR tcTypeID ) { m_TypeIDPtr.SetID( tcTypeID ); }
	void SetVersionID( LPCTSTR tcVersionID ) { m_VersionIDPtr.SetID( tcVersionID ); }
	void SetPriorityLevel( int iPriority ) { if( iPriority > 0 && iPriority <= 11 ) {  m_iPriorityLevel = iPriority; } }
	void SetComment( _string strComment ) { m_strComment = strComment; }
	void SetProdPicID( _string strProdPicID ) { m_strProdPicID = strProdPicID; }
	void SetAccessGroupID( _string strAccessGroupID ) { m_AccessGroupIDPtr.SetID( strAccessGroupID ); }
	void SetAccessGroupDB( CDataBase *pclDataBase ) { if( pclDataBase != m_AccessGroupIDPtr.DB ) { m_AccessGroupIDPtr.DB = pclDataBase; Modified(); } }
	void SetPartOfaSet( ePartOfaSet flag ) { if( flag != m_ePartOfaSet ) { m_ePartOfaSet = flag; Modified(); } }
	void SetTableSetID( _string ID ) { m_strTableSetID = ID; }
	virtual void SetPrice( double dPrice ) { m_dPrice = dPrice; Modified(); }

	// It's an helper for the 'Copy' method.
	void SetSortingKeyAsLL( CProductSortKey::sLL rSortingKey ) { m_SortingKey = rSortingKey; }

	// HYS-1018: for accessory
	void SetSeriesID( LPCTSTR strSeriesID );
	void SetBoolAttached( bool bAttached ) { m_bAttached = bAttached; }
	void SetProductUseFlag( int iProdUseFlag ) { m_iProductUseFlag = iProdUseFlag; }
	void SetAccessTypeID( LPCTSTR tcAccessTypeID ) { m_AccessTypeIDPtr.SetID( tcAccessTypeID ); }
	void SetAccessFamilyID( LPCTSTR tcAccessFamilyID ) { m_AccessFamilyIDPtr.SetID( tcAccessFamilyID ); }

	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual LPCTSTR GetArtNum( bool bWithoutConnection = false ) { return GetBodyArtNum(); }

#ifndef TACBX 
	// Old and new Cat can exist. If The new cat doesn't exist yet, verify the old one exist in the folder.
	// If one Cat exist it returns true.
	bool VerifyOneCatExist();
#endif

	void ApplyComment( _string *pstrComment );

	bool IsPartOfaSet() { return ePartOfaSetNo != m_ePartOfaSet; }

	// HYS-1018: for accessory
	bool IsAttached() { return m_bAttached; }
	bool IsAnAccessory() { return ( ( m_iProductUseFlag == eAccessory ) || ( m_iProductUseFlag == eBoth ) ); }
	// Return non zero if this object is priced.
	virtual int IsPriced() { return 1; }

	// Return sorting key based on index of Type, Family, PN, and PriorityLevel.
	// Lowest = highest priority.
	virtual double GetSortingKey( CProductSortKey::CPSKMask *pclMask = NULL );
	virtual void SetSortingKey( CProductSortKey::sLL SKey ) { m_SortingKey = SKey; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );

#ifndef TACBX
	virtual void DeleteArticle( void );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	virtual CheckFilterReturnValue CheckFilter( CFilterTab *pclFilter, CString strFilter );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CProductSortKey::sLL m_SortingKey;

// Private variables.
private:
	TCHAR m_tcArtNum[_PRODUCT_REF_LENGTH + 1];			// Product article number.
	TCHAR m_tcName[_PRODUCT_NAME_LENGTH + 1];			// Product name.
	IDPTR m_TypeIDPtr;									// Product type IDPTR.
	IDPTR m_FamilyIDPtr;								// Product family IDPTR.
	IDPTR m_VersionIDPtr;								// Product version IDPTR.
	int m_iPriorityLevel;								// Prior. level for select. [1,10].
	_string m_strComment;								// Product comment.
	_string m_strProdPicID;								// Product image ID.
	IDPTR m_AccessGroupIDPtr;
	ePartOfaSet m_ePartOfaSet;							// True when the valve is a part of a set.
	_string m_strTableSetID;								// ID of the corresponding table set.
	double m_dPrice;									// Product price.
	_string *m_pStr;
	// HYS-1018: New CDB_Product to include old CDB_Accessory
	int m_iProductUseFlag;								// Product use: 0 if only Product, 1 if Accessory, 2 if both
	IDPTR m_AccessTypeIDPtr;								// Product type IDPTR.
	IDPTR m_AccessFamilyIDPtr;								// Product family IDPTR.
	// for accessory
	IDPTR m_SeriesIDPtr;
	bool m_bAttached;
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_TAProduct
//
//	new:CDB_TAProduct(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//		BodyMaterialID			Identifier of the body material
//		SizeID					Identifier of the corresp. valve size
//		ConnectID				Identifier of the (inlet/pipe side) connection
//		ConnTabID				Identifier of the connection table, can be NO_ID
//		Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//		CBIType					Type of the valve as used in the CBI II
//		CBISize					Size of the valve as used in the CBI II
//		CBISizeInch				Size of the valve in inch as used in the CBI II
//		CBICompatibilityIndex	CBI compatibility Index
//		CBIVersion				CBI Version
//		Dpmax					Maximum Dp of the valve
//		MeasPt					Measuring points
//		MeasurableData			Measurable data
//		InletSizeID				Identifier of the inlet size ID
//		OutletSizeID			Identifier of the outlet size ID
//	}
//
///////////////////////////////////////////////////////////////////////////////////

#define _CBI_TYPE_LENGTH	15
#define _CBI_SIZE_LENGTH	15
#define _CBI_VERSION_LENGTH 6

class CDB_TAProduct : public CDB_Product
{
	DEFINE_DATA( CDB_TAProduct )
	
protected:
	CDB_TAProduct( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_TAProduct() {}

// Public enumeration defines.
public:
// Measuring capacity should be a geometric progression 1,2,4,8,...
//## eMeasPointInlet	=	0x01
//## eMeasPointOutlet	=	0x02
//
//## eMeasDataDP		=	0x01
//## eMeasDataFlow		=	0x02
//## eMeasDataDPSignal	=	0x04
	enum eMeasPt
	{
		empNone		= 0x00,
		empInlet	= 0x01,
		empOutlet	= 0X02,
		empStatOut	= 0x04,
		empPplus	= 0x08,
		empLast		= 0x10
	};

	enum eMeasurableData
	{
		emdNone		= 0x00,
		emdDp		= 0x01,
		emdQ		= 0x02,
		emdDpSignal	= 0x04,
		emdDpAvail	= 0x08,
		emdDpStab	= 0x10,
		emdDpCAlwaysOn = 0x20,
		emdLast = 0x40
	};

// Public methods.
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
 	const IDPTR &GetBodyMaterialIDPtr();										// IDPTR of Body material.
	LPCTSTR GetBodyMaterial();													// Body material.
	LPCTSTR GetBodyMaterialID() { return m_BodyMaterialIDPtr.ID; }				// Body material ID.

	const IDPTR &GetSizeIDPtr(); 												// IDPTR of corresponding valve size.
	LPCTSTR GetSize();															// Corresponding valve size name.
	LPCTSTR GetSizeID() { return m_SizeIDPtr.ID; }								// Corresponding valve size ID.
	int GetSizeKey();															// Corresponding valve size ordering key.
	
	const IDPTR &GetConnectIDPtr();												// IDPTR of connection.
	LPCTSTR GetConnect();														// Connection name.
	LPCTSTR GetConnectID() { return m_ConnectIDPtr.ID; }						// Connection ID.
	
	const IDPTR &GetConnTabIDPtr();												// IDPTR of connection table.
	LPCTSTR GetConnTabID() { return m_tcConnTabID; }							// Connection table ID, can be NO_ID.

	const IDPTR &GetConn2IDPtr();												// IDPTR of outlet/body side connection.
	LPCTSTR GetConnect2();														// Outlet connection name.
	LPCTSTR GetConn2ID() { return m_tcConn2ID; }								// Outlet/body side connection ID.

	// Retrieve article number that can be eventually composed by body + inlet + outlet references.
	// Param: if 'fWithoutConnection' is set to 'true', we send back article number without any connection reference.
	virtual LPCTSTR GetArtNum( bool fWithoutConnection = false );

	LPCTSTR GetCBIType() { return m_tcCBIType; }								// CBI type.
	
	LPCTSTR GetCBISize() { return m_tcCBISize; }								// CBI size.
	
	LPCTSTR GetCBISizeInch() { return m_tcCBISizeInch; }						// CBI size in inch.

	int GetCBICompatibilityIndex() { return m_iCBICompatibilityIndex; }			// CBI compatibility index.
	
	LPCTSTR GetCBIVersion() { return m_tcCBIVersion; }							// CBI version.

	double GetDpmax();

	unsigned GetMeasPt() { return m_uMeasPt; }
	unsigned GetMeasurableData() { return m_uMeasurableData; }


	// Retrieve ID on actuator group.
	virtual IDPTR GetActuatorGroupIDPtr( void ) { return _NULL_IDPTR; }
	
	
	// Retrieve ID on adapter group.
	virtual IDPTR GetAdapterGroupIDPtr( void ) { return _NULL_IDPTR; }

	// Inlet size.
	LPCTSTR GetInletSizeStr( void );
	const IDPTR &GetInletSizeIDPtr( void );

	// Outlet size.
	LPCTSTR GetOutletSizeStr( void );
	const IDPTR &GetOutletSizeIDPtr( void );

	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetBodyMaterialID(LPCTSTR ID) { m_BodyMaterialIDPtr = _NULL_IDPTR; _tcsncpy_s( m_BodyMaterialIDPtr.ID, SIZEOFINTCHAR( m_BodyMaterialIDPtr.ID ), ID, SIZEOFINTCHAR( m_BodyMaterialIDPtr.ID ) - 1 ); }

	LPCTSTR SetSizeID(LPCTSTR);		// Set the SizeID & cancels undesirable white spaces.
	void SetConnectID( LPCTSTR tcConnectID ) { m_ConnectIDPtr.SetID( tcConnectID ); };
	void SetConnTabID( LPCTSTR tcConnTabID ) { memset( m_tcConnTabID, 0, sizeof( m_tcConnTabID ) ); _tcsncpy_s( m_tcConnTabID, SIZEOFINTCHAR( m_tcConnTabID ), tcConnTabID, SIZEOFINTCHAR( m_tcConnTabID ) - 1 ); };
	void SetConn2ID( LPCTSTR tcConn2ID ) { memset( m_tcConn2ID, 0, sizeof( m_tcConn2ID ) ); _tcsncpy_s( m_tcConn2ID, SIZEOFINTCHAR( m_tcConn2ID ), tcConn2ID, SIZEOFINTCHAR( m_tcConn2ID ) - 1 ); };
	LPCTSTR SetCBIType( LPCTSTR );		// Set the CBIType & trims undesirable white spaces.
	void SetCBISize( LPCTSTR tcCBISizeID ){ memset( m_tcCBISize, 0, sizeof( m_tcCBISize ) ); _tcsncpy_s( m_tcCBISize, SIZEOFINTCHAR( m_tcCBISize ), tcCBISizeID, SIZEOFINTCHAR( m_tcCBISize ) - 1 ); }
	void SetCBISizeInch( LPCTSTR tcCBISizeInchID ){ memset( m_tcCBISizeInch, 0, sizeof( m_tcCBISizeInch ) ); _tcsncpy_s( m_tcCBISizeInch, SIZEOFINTCHAR( m_tcCBISizeInch ), tcCBISizeInchID, SIZEOFINTCHAR( m_tcCBISizeInch ) - 1 ); }
	void SetCBICompatibilityIndex( int iCompIndex ) { m_iCBICompatibilityIndex = iCompIndex; }
	void SetCBIVersion( LPCTSTR tcCBIVersionID ){ memset( m_tcCBIVersion, 0, sizeof( m_tcCBIVersion ) ); _tcsncpy_s( m_tcCBIVersion, SIZEOFINTCHAR( m_tcCBIVersion ), tcCBIVersionID, SIZEOFINTCHAR( m_tcCBIVersion ) - 1 ); }
	void SetDpmax( double dDp ) { m_dDpmax = dDp; }
	void SetMeasPt( unsigned uMeasPt ) { m_uMeasPt = uMeasPt; }
	void SetMeasurableData( unsigned uMeasurableData ) { m_uMeasurableData = uMeasurableData; }
	void SetInletSizeID( LPCTSTR tcInletSizeID ) { m_InletSizeIDPtr.SetID( tcInletSizeID ); }
	void SetOutletSizeID( LPCTSTR tcOutletSizeID ) { m_OutletSizeIDPtr.SetID( tcOutletSizeID ); }

	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Test if an valid ID is stored in ConnTabID field.
	bool IsConnTabUsed();

	// return true if DpSignalExist.
	virtual bool IsKvSignalEquipped( void ) { return false; }
	
	// Return characteristic of the valve.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic( void ) { return NULL; }
	
	// Return Valve Char data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	virtual CData *GetValveCharDataPointer( void ) { return NULL; }

	// Retrieve the fully qualified pointer to the control properties.
	virtual CDB_ControlProperties *GetCtrlProp() { return NULL; }

	virtual double GetKvSignal() { return 0.0; }
	
	// Return sorting key based on index of Type, Family, Bdy, Connection, Version, PN, and PriorityLevel.
	// Lowest = highest priority.
	virtual double GetSortingKey( CProductSortKey::CPSKMask* pclMask = NULL );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void CheckRedefinition( CRedefineTab *pclRedef, CString strRedefinition );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	virtual CheckFilterReturnValue CheckFilter( CFilterTab *pclFilter, CString strFilter );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	IDPTR m_BodyMaterialIDPtr;							// Body material IDPTR.
	IDPTR m_SizeIDPtr;									// Corresponding valve size IDPTR.
	IDPTR m_ConnectIDPtr;								// Connection (inlet/pipe side) IDPTR.
	TCHAR m_tcConnTabID[_ID_LENGTH + 1];				// Connection table ID, can be NO_ID.
	TCHAR m_tcConn2ID[_ID_LENGTH + 1];					// Connection outlet/body side ID, can be NO_ID.
	TCHAR m_tcCBIType[_CBI_TYPE_LENGTH + 1];
	TCHAR m_tcCBISize[_CBI_SIZE_LENGTH+1];
	TCHAR m_tcCBISizeInch[_CBI_SIZE_LENGTH + 1];
	int m_iCBICompatibilityIndex;						// Compatibility index.
	TCHAR m_tcCBIVersion[_CBI_VERSION_LENGTH + 1];		// CBI version.
	double m_dDpmax;									// Maximum Dp.
	unsigned m_uMeasPt;									// Measuring points.
	unsigned m_uMeasurableData;							// Measurable data.
	IDPTR m_InletSizeIDPtr;								// Identifier of the inlet size ID.
	IDPTR m_OutletSizeIDPtr;							// Identifier of the outlet size ID.
};


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DpSensor
//
//	new:CDB_DpSensor(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the product.
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			TableSetID				ID of the table containing the set
//			Price					Price of the product
//			ProductUseFlag			#eProduct, #eAccessory, #eBoth Integer to indicate if the product is only a product, only accessory or both
//			AccessorySeriesID		Identifier of the series of the accessory (mainly used in DlgDirSel)
//			isAttached				0/1: 1 if accessory cannot be ordered separately (for DlgDirSel)
//			AccTypeID				Identifier of the accessory type if #eBoth 
//			AccFamilyID				Identifier of the accessory family if #eBoth
//		}
//		MinMeasurableDp			Minimum differential pressure that the instrument can read.
//		MaxMeasurableDp			Maximum differential pressure that the instrument can read.
//      BurstPressure			Max. differential pressure.
//		OutputSignals			List of accepted output control signals separated by "\"
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CTableSet;
class CDB_DpSensor : public CDB_Product
{
	DEFINE_DATA( CDB_DpSensor )

protected:
	CDB_DpSensor( CDataBase *pDataBase, LPCTSTR ptcStrID );
	virtual ~CDB_DpSensor() {}

// Public methods.
public:
	CString GetFullName();
	double GetMinMeasurableDp() { return m_dMinMeasurableDp; }
	double GetMaxMeasurableDp() { return m_dMaxMeasurableDp; }
	double GetBurstPressure() { return m_dBurstPressure; }

	//	- Returns a formated string with Dpl range if exists.
	_string GetFormatedDplRange( bool bWithUnit = true );

	bool IsOutputSignalAvailable( IDPTR IDPtrOutput );
	unsigned int GetNumOfOutputSignalsIDptr() { return (int)m_OutputSignals.GetCount(); }
	IDPTR GetOutputSignalsIDPtr( unsigned int uiIndex );
	CString GetOutputSignalsStr( CString strSeparator = L";" );

	void SetMinMeasurableDp( double dMinMeasurableDp ) { m_dMinMeasurableDp = dMinMeasurableDp; }
	void SetMaxMeasurableDp( double dMaxMeasurableDp ) { m_dMaxMeasurableDp = dMaxMeasurableDp; }
	void SetBurstPressure( double dBurnPressure ) { m_dBurstPressure = dBurnPressure; }
	void SetOutputSignals( CArray<IDPTR> &arOutputSignals );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

private:
	double m_dMinMeasurableDp;
	double m_dMaxMeasurableDp;
	double m_dBurstPressure;
	CArray<IDPTR> m_OutputSignals;
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Vessel
//
//	new:CDB_Vessel(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					0						isDeleted
//					1						isAvailable
//					0						isHidden
//					1						isFix
//					0						Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0						Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0						Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax		in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID				Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//		NominalVolume			double (m3)	
//		FactoryP0				double (Pa)
//		ConnectID				string ID
//		SizeID					string ID
//		Height					double (m)
//		Diameter				double (m)	
//		Weight					double (kg)
//		FloorStanding			true/false
//		Compatibilityflags		can be combined #Statico | #Compresso | #Transfero | #Transfero-TI
//		SecondaryVesselID		Identifier of the associated secondary Vssl
//      PlenoID					Identifier of the Pleno group that match the vessel.
//      PSch                    SWKI HE301-01: Pressure up to which the expansion vessel must not subject to authorization, according to Swiss directive SICC 93-1.
//      AcceptTecBoxOnTop		0/1
//	}
//
//  MinimumVesselVolume is defined with a range of pressure and the minimum vessel volume needed:
//
//			p00 \ p01 \ V0 \ p10 \ p11 \ V1 ...
//
//  Means: when p > p00 and p <= p01 the Vento can work with vessel from V0 liter.
//         when p > p10 and p <= p11 the Vento can work with vessel from V1 liter. 
//
//  Example: For Vento Connect we need at least a vessel of 80l:
//
//			0 \ -1 \ 0.08
//   
//  Means: when p > 0 Pa and no limit (except the Vento and vessel limit) Vento can work with vessel from 80l.
//
//  Example: For Vento VI we have two pressure ranges:
//
//         0 \ 1e5 \ 0.15 \ 1e5 \ -1 \ 0.3
//
//  Means: when p > 0 bar and p <= 10 bar the Vento VI can work with vessel from 150l.
//         when p > 10 bar and no limit (except the Vento and vessel limit) Vento VI can work with vessel from 300l.
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_Vessel : public CDB_Product
{
	DEFINE_DATA( CDB_Vessel )

protected:
	CDB_Vessel( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Vessel() {}

// Public enumeration defines.
public:
 	// ePMCompFlags should be a geometric progression 1,2,4,8,...
	typedef enum ePMCompFlags
	{
		ePMComp_None = 0x00,
		ePMComp_Statico = 0x01,
		ePMComp_Compresso = 0x02,
		ePMComp_Transfero = 0x04,
		ePMComp_TransferoTI = 0x08,
		ePMComp_VsslIntCoating = 0x10,
		ePMComp_Mask = 0x1F,
		ePMComp_Latest = 0x20
	};

	typedef enum VsslType
	{
		eVsslType_Statico,
		eVsslType_StatMbr,
		eVsslType_Prim,
		eVsslType_Sec,
		eVsslType_Interm,
		eVsslType_Aqua,
		eVsslType_Last
	};

// Public methods.
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	double GetNominalVolume() const { return m_dNominalVolume; }
	double GetFactoryP0() const { return m_dFactoryP0; }
	LPCTSTR GetConnect();
	const IDPTR &GetConnectIDPtr();
	LPCTSTR GetSize();
	const IDPTR &GetSizeIDPtr();
	const IDPTR &GetSecVsslIDPtr();
	double GetHeight() const { return m_dHeight; }
	double GetDiameter() const { return m_dDiameter; }
	double GetWeight() const { return m_dWeight; }
	int GetInstallationLayout() const { return m_iInstallationLayout; }
	int GetInstallationLayoutPictureID();
	int GetCompatibilityflags() const { return m_Compatibilityflags; }

	VsslType GetVesselType();

	// We don't have a property to tell if a vessel is used in a normal way or as a buffer.
	// This information must come frome outside. The case is for the Transfero with SH vessel for example.
	CString GetVesselTypeStr( bool bIsIntegratedIsBuffer = false );

	CDB_Vessel *GetAssociatedSecondaryVessel();	// Return Secondary Vessel
	IDPTR GetPlenoIDPtr();
	bool IsPrimaryVesselIntegrated( void );
	double GetPSch() const { return m_dPSch; }

	// HYS-1374.
	bool IsAcceptTecBoxOnTop() { return m_bAcceptTecboxOnTop; }

	bool IsInstallationLayoutWallHanged() { return ( CDB_Product::eilWallHanged == ( CDB_Product::eilWallHanged & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutVesselTop() { return ( CDB_Product::eilVesselTop == ( CDB_Product::eilVesselTop & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutFloorStanding() { return ( CDB_Product::eilFloorStanding == ( CDB_Product::eilFloorStanding & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutFloorStandingNextTo() { return ( CDB_Product::eilFloorStandingNextTo == ( CDB_Product::eilFloorStandingNextTo & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutFloorStandingInFront() { return ( CDB_Product::eilFloorStandingInFront == ( CDB_Product::eilFloorStandingInFront & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutIntegrated() { return ( CDB_Product::eilIntegrated == ( CDB_Product::eilIntegrated & m_iInstallationLayout ) ) ? true : false; }
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetNominalVolume( double dNominalVolume ) { m_dNominalVolume = dNominalVolume; }
	void SetFactoryP0( double dFactoryP0 ) { m_dFactoryP0 = dFactoryP0; }
	void SetConnectID( LPCTSTR tcConnectID ) { m_ConnectIDPtr.SetID( tcConnectID ); }
	void SetSizeID( LPCTSTR tcSizeID ) { m_SizeIDPtr.SetID( tcSizeID ); }
	void SetSecVsslID( LPCTSTR tcSecVsslID ) { m_SecVsslIDPtr.SetID( tcSecVsslID ); }
	void SetHeight( double dHeight ) { m_dHeight = dHeight; }
	void SetDiameter( double dDiameter ) { m_dDiameter = dDiameter; }
	void SetWeight( double dWeight ) { m_dWeight = dWeight; }
	void SetInstallationLayout( int iInstallationLayout ) { m_iInstallationLayout = iInstallationLayout; }
	void SetCompatibilityflags( int iCompatibilityflags ) { m_Compatibilityflags = iCompatibilityflags; }
	void SetPlenoID( LPCTSTR tcPlenoID ) { m_PlenoIDPtr.SetID( tcPlenoID ); }
	void SetPSch( double dPSch ) { m_dPSch = dPSch; }

	// HYS-1374.
	void SetAcceptTecBoxOnTop( bool bAcceptTecboxOnTop ) { m_bAcceptTecboxOnTop = bAcceptTecboxOnTop; }
 	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Test one compatibility flag.
	bool IsCompatible( ePMCompFlags CmpFlg );

	// Test 'And' combination of compatibility flag.
	bool IsCompatibleAND( int iCmpFlg );

	// Test 'OR' combination of compatibility flag.
	bool IsCompatibleOR( int iCmpFlg );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dNominalVolume;				// NominalVolume		double (m3) 
	double m_dFactoryP0;					// FactoryP0			double (Pa) 
	IDPTR m_ConnectIDPtr;					// ConnectID			string ID   
	IDPTR m_SizeIDPtr;						// SizeID				string ID   
	IDPTR m_SecVsslIDPtr;					// Secondary vessel		IDPTR
	double m_dHeight;						// Height				double (m)  
	double m_dDiameter;						// Diameter				double (m)	
	double m_dWeight;						// Weight				double (kg) 
	int m_iInstallationLayout;	
	int m_Compatibilityflags;				// Can be combined #ePMComp_Statico | #ePMComp_Compresso | #ePMComp_Transfero | #ePMComp_TransferoTI | #ePMComp_VsslIntCoating
	IDPTR m_PlenoIDPtr;
	double m_dPSch;							// SWKI HE301-01: Pressure up to which the expansion vessel must not subject to authorization, according to Swiss directive SICC 93-1.
	bool m_bAcceptTecboxOnTop;				// HYS-1374: 0/1
};
#endif // TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_TecBox
//
//	new:CDB_TecBox(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//		IPxx					Enclosure protection 
//		Power supply			List of accepted Power supplies separated by "\"
//		Frequency				Accepted frequency
//		Power					Power consumption
//		SoundPressureLevel		Sound pressure level only for compresso and vento
//		NbrPumpCompressor		0/1/2 ( 0 for CX )
//	    PumpCompressCurveID		Pump/compressor curve ID.
//		Accuracy				in Pascal
//		InstallationLayout		#eWallHanged, #eFloorStanding, #eFloorStandingNextTo, #eFloorStandingInFront, #eVesselTop or #eIntegrated
//		Height					double (m)
//		Width					double (m)	
//		Depth					double (m)
//		Weight					double (kg)
// 		Functions 				#ePMFunc_None/#ePMFunc_Degasing/#ePMFunc_WaterMakeUp/#ePMFunc_Compressor/#ePMFunc_Pump should be a geometric progression 1,2,4,8,...
// 		FunctionsExclusion		#ePMFunc_None/#ePMFunc_Degasing/#ePMFunc_WaterMakeUp/#ePMFunc_Compressor/#ePMFunc_Pump should be a geometric progression 1,2,4,8,...
// 		TecBox Variant			#ePMVariant_None/#ePMVariant_Cooling/#ePMVariant_EcoAuto/#ePMVariant_BreakTank/#ePMVariant_VacuumDegassing/
//								#ePMVariant_ExternalAir/#ePMVariant_IntBufferVssl/#ePMVariant_HighFlowTemp
//		TmaxWaterMakeUp			Max admissible temperature for WM (~30�C)
//		PlenoID					Identifier of the Pleno group that match the Tecbox.
//		IntegratedVesselID		Identifier of the buffer vessel ID if fixed for Transfero or integrated vessel ID if fixed for Simply Compresso.
//								Can be followed by \ x where x indicated the maximum number of vessel possible in the TecBox.
//      WaterTreatmentCombID	Identifier of the different combination of the water treatment.
//      MaxSecondaryVessels		Maximum allowed secondary vessel. If -1 we take the 'm_iMaxNumberOfVesselsInParallel' - 1 from the technical parameters.
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_TecBox : public CDB_Product
{
	DEFINE_DATA( CDB_TecBox )

protected:
	CDB_TecBox( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_TecBox() {}

// Public enumeration defines.
public:
	typedef enum TecBoxType
	{
		etbtNone = 0x01,
		etbtCompresso = 0x02,
		etbtTransfero = 0x04,
		etbtTransferoTI = 0x08,
		etbtVento = 0x10,
		etbtPleno = 0x20,
		epmcLatest = 0x40
	};

	// TecBox functions 	#ePMFunc_None/#ePMFunc_Degasing/#ePMFunc_WaterMakeUp/#ePMFunc_Compressor/#ePMFunc_Pump should be a geometric progression 1,2,4,8,...   
	typedef enum TBFunctions
	{
		ePMFunc_None = 0,
		ePMFunc_First = 1,
		ePMFunc_Degasing = ePMFunc_First,
		ePMFunc_WaterMakeUp = 2,
		ePMFunc_Compressor = 4,
		ePMFunc_Pump = 8,
		ePMFunc_Last = ePMFunc_Pump,
	};
	
	// HYS-1121: Add ePMVariant_WMProtectionConnection variant for Pleno which have connection for pretection module
	// TecBox variant		#ePMVariant_None/#ePMVariant_Cooling/#ePMVariant_EcoAuto/#ePMVariant_BreakTank/#ePMVariant_VacuumDegassing/
	//						#ePMVariant_ExternalAir/#ePMVariant_IntBufferVssl/#ePMVariant_HighFlowTemp/#ePMVariant_IntegratedPrimaryVessel/#ePMVariant_WMProtectionConnection
	typedef enum TBVariant
	{
		ePMVariant_None = 0,
		ePMVariant_Cooling = 1,
		ePMVariant_EcoAuto = 2,
		ePMVariant_BreakTank = 4,
		ePMVariant_VacuumDegassing = 8,
		ePMVariant_ExternalAir = 16,
		ePMVariant_IntBufferVssl = 32,
		ePMVariant_HighFlowTemp = 64,
		ePMVariant_IntegratedPrimaryVessel = 128,
		ePMVariant_WMProtectionConnection = 256,
		ePMVariant_Last
	};

	typedef enum TBIntegratedVesselType
	{
		ePMIntVesselType_None,
		ePMIntVesselType_Buffer,
		ePMIntVesselType_Primary
	};

// Public methods.
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	double GetHeight() const { return m_dHeight; }
	double GetWeight() const { return m_dWeight;}
	double GetDepth() const { return m_dDepth; }
	double GetWidth() const { return m_dWidth; }
	int GetInstallationLayout() const { return m_iInstallationLayout;	}
	virtual int GetInstallationLayoutPictureID();
	double GetAccuracy() const { return m_dAccuracy; }
	double GetSoundPressureLevel() const { return m_dSoundPressureLevel; }
	double GetPower() const { return m_dPower; }
	int GetNbrPumpCompressor() const { return m_iNbrPumpCompressor; }
	
	CString GetIPxx() {	return CString( m_IPxx ); }

	bool IsPowerSupplyAvailable( IDPTR idptr );
	unsigned int GetNumOfPowerSupplyIDptr() { return (int)m_arPowerSupplyIDPtr.GetCount(); }
	IDPTR GetPowerSupplyIDPtr( unsigned int index );
	CString GetPowerSupplyStr( CString strSeparator = _T("; ") );

	IDPTR GetFrequencyIDPtr();

	IDPTR GetPumpComprCurveIDPtr();

	int GetFunctions() const { return m_iFunctions; }
	int GetFunctionsExcluded() const { return m_iFunctionsExcluded; }
	int GetNumberOfFunctions( void );
	CString GetFunctionsStr( CString strTecboxType = _T("") );

	int GetTecBoxVariant() const { return m_iTecBoxVariant; }
	CDB_TecBox::TecBoxType GetTecBoxType();

	// It's for Transfero/Compresso combined with a water make-up device.
	// For a Pleno, this value is the same as 'CDB_HydroThing::Tmax'.
	double GetTmaxWaterMakeUp() const {	return m_dTmaxWaterMakeUp; }

	IDPTR GetPlenoIDPtr();

	IDPTR GetIntegratedVesselIDPtr();
	int GetMaximumSecondaryVessels();

	IDPTR GetWaterTreatmentCombIDPtr( void );

	bool IsInstallationLayoutWallHanged() { return ( CDB_Product::eilWallHanged == ( CDB_Product::eilWallHanged & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutVesselTop() { return ( CDB_Product::eilVesselTop == ( CDB_Product::eilVesselTop & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutFloorStanding() { return ( CDB_Product::eilFloorStanding == ( CDB_Product::eilFloorStanding & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutFloorStandingNextTo() { return ( CDB_Product::eilFloorStandingNextTo == ( CDB_Product::eilFloorStandingNextTo & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutFloorStandingInFront() { return ( CDB_Product::eilFloorStandingInFront == ( CDB_Product::eilFloorStandingInFront & m_iInstallationLayout ) ) ? true : false; }
	bool IsInstallationLayoutIntegrated() { return ( CDB_Product::eilIntegrated == ( CDB_Product::eilIntegrated & m_iInstallationLayout ) ) ? true : false; }

	bool IsFctDegassing();
	bool IsFctWaterMakeUp();
	bool IsFctCompressor();
	bool IsFctPump();

	bool IsFctExcludedDegassing();
	bool IsFctExcludedWaterMakeUp();
	bool IsFctExcludedCompressor();
	bool IsFctExcludedPump();

	bool IsVariantCooling();
	bool IsVariantEcoAuto();
	bool IsVariantBreakTank();
	bool IsVariantVacuumDegassing();
	bool IsVariantExternalAir();
	bool IsVariantIntegratedBufferVessel();
	bool IsVariantHighFlowTemperature();
	bool IsVariantIntegratedPrimaryVessel();
	bool IsVariantWMProtectionConnection();

	TBIntegratedVesselType GetIntegratedVesselType();

	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
 	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void SetNbrPumpCompressor( int val ) { m_iNbrPumpCompressor = val; }
	void SetPower( double val ) { m_dPower = val; }
	void SetSoundPressureLevel( double val ) { m_dSoundPressureLevel = val; }
	void SetAccuracy( double val ) { m_dAccuracy = val;	}
	void SetHeight( double val ) { m_dHeight = val; }
	void SetWeight( double val ) { m_dWeight = val; }
	void SetWidth( double val ) { m_dWidth = val; }
	void SetDepth( double val ) { m_dDepth = val; }
	void SetIPxx( LPCTSTR tcIPxx ) { _tcsncpy_s( m_IPxx, SIZEOFINTCHAR( m_IPxx ), tcIPxx, SIZEOFINTCHAR( m_IPxx ) - 1 ); }
	void SetFrequencyID( LPCTSTR tcFrequencyID ) { m_FrequencyIDPtr.SetID( tcFrequencyID ); }
	void SetPumpCompressCurveID( LPCTSTR tcPumpCompressCurveID ) { m_PumpCompressCurveIDPtr.SetID( tcPumpCompressCurveID ); }
	void SetPowerSupplyIDPtr( CArray<IDPTR> &arPowerSupplyIDPtr );
	void SetInstallationLayout( int iInstallationLayout ) { m_iInstallationLayout = iInstallationLayout; }
	void SetFunctions( int iFunctions ) { m_iFunctions = iFunctions; }
	void SetFunctionsExcluded( int iFunctionsExcluded ) { m_iFunctionsExcluded = iFunctionsExcluded; }
	void SetTecBoxVariant( int iTecBoxVariant ) { m_iTecBoxVariant = iTecBoxVariant; }
	void SetTmaxWaterMakeUp( double dTmaxWaterMakeUp ) { m_dTmaxWaterMakeUp = dTmaxWaterMakeUp; }
	void SetPlenoID( LPCTSTR tcPlenoID ) { m_PlenoIDPtr.SetID( tcPlenoID ); }
	void SetIntegratedVesselID( LPCTSTR tcIntegratedVesselID ) { m_IntegratedVesselIDPtr.SetID( tcIntegratedVesselID ); }
	void SetMaxSecondaryVessels( int iMaxSecondaryVessels ) { m_iMaxSecondaryVessels = iMaxSecondaryVessels; }
	void SetWaterTreatmentCombID( LPCTSTR tcWaterTreatmentCombID ) { m_WaterTreatmentCombIDPtr.SetID( tcWaterTreatmentCombID ); }


	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	int m_iNbrPumpCompressor;
	double m_dPower;
	double m_dSoundPressureLevel;
	double m_dAccuracy;
	double m_dHeight;
	double m_dWidth;
	double m_dDepth;
	double m_dWeight;
	TCHAR m_IPxx[8];
	IDPTR m_FrequencyIDPtr;
	IDPTR m_PumpCompressCurveIDPtr;
	CArray <IDPTR> m_arPowerSupplyIDPtr;
	int m_iInstallationLayout;
	int m_iFunctions;
	int m_iFunctionsExcluded;
	int m_iTecBoxVariant;
	double m_dTmaxWaterMakeUp;
	IDPTR m_PlenoIDPtr;
	IDPTR m_IntegratedVesselIDPtr;
	int m_iMaxSecondaryVessels;
	IDPTR m_WaterTreatmentCombIDPtr;
};
#endif // TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_TBPlenoVento
//
//	Inherited from CDB_TecBox
//
//	new:CDB_TBPlenoVento(ID)->tableID
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			IPxx					Enclosure protection 
//			Power supply			List of accepted Power supplies separated by "\"
//			Frequency				Accepted frequency
//			Power					Power consumption
//			SoundPressureLevel		Sound pressure level only for compresso and vento
//			NbrPumpCompressor		0/1/2 ( 0 for CX )
//			PumpCompressCurveID		Pump/compressor curve ID.
//			Accuracy				in Pascal
//			InstallationLayout		#eWallHanged, #eFloorStanding, #eFloorStandingNextTo, #eFloorStandingInFront, #eVesselTop or #eIntegrated
//			Height					double (m)
//			Width					double (m)	
//			Depth					double (m)
//			Weight					double (kg)
//			Functions 				#ePMFunc_None/#ePMFunc_Degasing/#ePMFunc_WaterMakeUp/#ePMFunc_Compressor/#ePMFunc_Pump should be a geometric progression 1,2,4,8,...
// 			TecBox Variant			#ePMVariant_None/#ePMVariant_Cooling/#ePMVariant_EcoAuto/#ePMVariant_BreakTank/#ePMVariant_VacuumDegassing/
//									#ePMVariant_ExternalAir/#ePMVariant_IntBufferVssl/#ePMVariant_HighFlowTemp
//			TmaxWaterMakeUp			Max admissible temperature for WM (~30�C)
//			PlenoID					Identifier of the Pleno group that match the Transfero/Compresso TecBox.
//			IntegratedVesselID			Identifier of the buffer vessel ID if fixed.
//		}
//		SystemVolume										double (m3) for CPV,TV,...
//		WorkingPressRangeMin \ WorkingPressRangeMax			double Dpp for Pleno
//		FlowWaterMakeUp										double (VNS in DE)
//		Kvs													double (Pleno P, Pi)
//		Refillable					#ePMPR_RefillNone/#ePMPR_RefillOptional/#ePMPR_RefillMandatory
//		RefillGroupID				Identifier of the Pleno Refill group that match the Pleno.
//      MinimumVesselVolume			Identify what is the minimum vessel volume allowed with this Vento (See below to complete details)
//	}
//
//  MinimumVesselVolume is defined with a range of pressure and the minimum vessel volume needed for this range:
//
//			p00 \ p01 \ V0 \ p10 \ p11 \ V1 ...
//
//  Means: when p > p00 and p <= p01 the Vento can work with vessel from V0 liter.
//         when p > p10 and p <= p11 the Vento can work with vessel from V1 liter. 
//
//  Example: For Vento Connect we need at least a vessel of 80l:
//
//			0 \ -1 \ 0.08
//   
//  Means: when p > 0 Pa and no limit (except the Vento and vessel limits) Vento can work with vessel from 80l.
//
//  Example: For Vento VI we have two pressure ranges:
//
//         0 \ 1e5 \ 0.15 \ 1e5 \ -1 \ 0.3
//
//  Means: when p > 0 bar and p <= 10 bar the Vento VI can work with vessel from 150l.
//         when p > 10 bar and no limit (except the Vento and vessel limits) Vento VI can work with vessel from 300l.
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_TBPlenoVento : public CDB_TecBox
{
	DEFINE_DATA( CDB_TBPlenoVento )

protected:
	CDB_TBPlenoVento( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_TBPlenoVento() {}

// Public enumeration defines.
public:
	// Refillable 	#ePMPR_RefillNone/#ePMPR_RefillOptional/#ePMPR_RefillMandatory
	typedef enum ePRRefillable
	{
		ePMPR_RefillNone = 0,
		ePMPR_RefillOptional = 1,
		ePMPR_RefillMandatory = 2
	};

	typedef struct _VesselVolumeLimit
	{
		double m_dPmin;
		double m_dPmax;
		double m_dMinimumVolume;
	}VesselVolumeLimit;

// Public methods.
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	double GetSystemVolume() const { return m_dSystemVolume; }
	double GetWorkingPressRangeMin() const { return m_dWorkingPressRangeMin; }
	double GetWorkingPressRangeMax() const { return m_dWorkingPressRangeMax; }
	_string GetWorkingPressureRange( void );
	double GetFlowWaterMakeUp() const { return m_dFlowWaterMakeUp; }
	double GetKvs() const {	return m_dKvs; }
	ePRRefillable GetRefillable() const { return m_eRefillable; }
	const IDPTR& GetRefillGroupIDPtr( void );
	double GetMinimumVesselVolume( double dP );

	// For 'CopyFrom'.
	CArray<VesselVolumeLimit> *GetMinimumVesselVolumeArray( void ) { return &m_arVesselVolumeLimits; }
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetSystemVolume( double dValue ) {	m_dSystemVolume = dValue; }
	void SetWorkingPressRangeMin( double dValue ) {	m_dWorkingPressRangeMin = dValue; }
	void SetWorkingPressRangeMax( double dValue ) {	m_dWorkingPressRangeMax = dValue; }
	void SetFlowWaterMakeUp( double dValue ) { m_dFlowWaterMakeUp = dValue;	}
	void SetKvs( double dValue ) { m_dKvs = dValue;	}
	void SetRefillable( ePRRefillable eRefillable ) { m_eRefillable = eRefillable; }
	void SetRefillGroupID( LPCTSTR tcRefillGroupID ) { m_RefillGroupIDPtr.SetID( tcRefillGroupID ); }
	void SetVesselVolumeLimits( CArray<VesselVolumeLimit> &arVesselVolumeLimits );
 	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Overrides 'CDB_TecBox' public virtual methods.
	virtual int GetInstallationLayoutPictureID();

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dSystemVolume;
	double m_dWorkingPressRangeMin;
	double m_dWorkingPressRangeMax;
	double m_dFlowWaterMakeUp;
	double m_dKvs;
	ePRRefillable m_eRefillable;
	IDPTR m_RefillGroupIDPtr;
	CArray<VesselVolumeLimit> m_arVesselVolumeLimits;
};
#endif // TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_AirVent
//
//	new:CDB_AirVent(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		Dpp			Max static pressure difference with the atmosphere
//	}
//
////////////////////////////////////////////////////////////////
class CDB_AirVent : public CDB_TAProduct
{
	DEFINE_DATA( CDB_AirVent )
	
// Constructor and destructor.
protected:
	CDB_AirVent( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_AirVent() {}

// Public methods.
public:
	double GetDpp() { return m_dDpp; }
	void SetDpp( double dDpp ) { m_dDpp = dDpp; }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dDpp;			// Max static pressure difference with the atmosphere.
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Separator
//
//	new:CDB_Separator(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		CharactID		Identifier of the separator QDp characteristic
//      NumberOfMagnet    The number of magnet. 0 for separator without magnet.
//	}
//
////////////////////////////////////////////////////////////////
class CDB_Separator : public CDB_TAProduct
{
	DEFINE_DATA( CDB_Separator )
	
// Constructor and destructor.
protected:
	CDB_Separator( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Separator() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the Q-Dp characteristic.
	CDB_QDpCharacteristic *GetQDpCharacteristic();
	int GetNumberOfMagnet() { return m_iNbOfMagnet; }

	// Set the Q-Dp characteristic.
	void SetQDpCharacteristicID( _string ID );
	void SetNumberOfMagnet( int iValue ){ m_iNbOfMagnet = iValue; }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	virtual void CrossVerifyID();
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	TCHAR m_QDpCharacteristicID[_ID_LENGTH + 1];				// Q-Dp Characteristic ID.
	CDB_QDpCharacteristic *m_pQDpCharacteristic;				// Pointer on Q-Dp characteristic.
	int m_iNbOfMagnet;											
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_PlenoRefill
//
//	new:CDB_PlenoRefill(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//		Functions				#ePRFunc_None/#ePRFunc_Soft/#ePRFunc_Desalin
//		Capacity				double ( l*�dH -> SI: m3*mol/m3 = mol)
//		MaxFlow					double (m3/s)
//      MaxSupplyWaterP			double (pa)
//      MinSupplyWaterP			double (pa)
//		InConnectID				string ID
//      InSizeID				string ID
//		OutConnectID			string ID
//      OutSizeID				string ID
//		Height					double (m)
//		WidthWoConnect			double (m)
//		WidthWConnect			double (m)	
//		Weight					double (kg)
//      DegasserCompatible		0/1 (For example: Pleno Refill 6000/12000 are not compatible with a Vento).
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_PlenoRefill : public CDB_Product
{
	DEFINE_DATA( CDB_PlenoRefill )

protected:
	CDB_PlenoRefill( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_PlenoRefill() {}

// Public enumeration defines.
public:
	// PlenoRefill functions 	#ePRFunc_None/#ePRFunc_Soft/#ePRFunc_Desalin
	typedef enum ePRFunctions
	{
		ePRFunc_None = 0,
		ePRFunc_Soft = 1,
		ePRFunc_Desalin = 2,
	};

// Public methods.
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	int GetFunctions() const { return m_iFunctions; }
	CString GetFunctionsStr( void );
	double GetCapacity( void ) const { return m_dCapacity; }
	CString GetCapacityStr( bool bWithUnit = false );
	
	// 'dTotalHardnessOfSystem' must be determined with 'CDB_TotalHardnessCharacteristic' (see explanation of this class).
	double GetMaxVolume( double dWaterHardness, double dTotalHardnessOfSystem );
	
	double GetMaxFlow( void ) const { return m_dMaxFlow; }
	double GetMaxSupplyWaterPressure( void ) const { return m_dMaxSupplyWaterPressure; }
	double GetMinSupplyWaterPressure( void ) const { return m_dMinSupplyWaterPressure; }
	CString GetSupplyWaterPressureRangeStr( void );
	LPCTSTR GetInConnectStr( void );
	const IDPTR &GetInConnectIDPtr( void );
	LPCTSTR GetInSizeStr( void );
	const IDPTR &GetInSizeIDPtr( void );
	CString GetInConnectSizeStr( void );
	LPCTSTR GetOutConnectStr( void );
	const IDPTR &GetOutConnectIDPtr( void );
	LPCTSTR GetOutSizeStr( void );
	const IDPTR &GetOutSizeIDPtr( void );
	CString GetOutConnectSizeStr( void );
	bool HasFlowLimiter( void ) const { return ( -1.0 != m_dMaxFlow ) ? true : false; }
	double GetHeight( void ) const { return m_dHeight; }
	double GetWidth( bool fWithConnection = true ) const;
	double GetWeight( void ) const { return m_dWeight; }
	bool IsDegasserCompatible( void ) const { return m_bDegasserCompatible; }
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetFunctions( int iFunctions ) { m_iFunctions = iFunctions; }
	void SetCapacity( double dCapacity ) { m_dCapacity = dCapacity; }
	void SetMaxFlow( double dMaxFlow ) { m_dMaxFlow = dMaxFlow; }
	void SetMaxSupplyWaterPressure( double dMaxSupplyWaterPressure ) { m_dMaxSupplyWaterPressure = dMaxSupplyWaterPressure; }
	void SetMinSupplyWaterPressure( double dMinSupplyWaterPressure ) { m_dMinSupplyWaterPressure = dMinSupplyWaterPressure; }
	void SetInConnectID( LPCTSTR tcInConnectID ) { m_InConnectIDPtr.SetID( tcInConnectID ); }
	void SetInSizeID( LPCTSTR tcInSizeID ) { m_InSizeIDPtr.SetID( tcInSizeID ); }
	void SetOutConnectID( LPCTSTR tcOutConnectID ) { m_OutConnectIDPtr.SetID( tcOutConnectID ); }
	void SetOutSizeID( LPCTSTR tcOutSizeID ) { m_OutSizeIDPtr.SetID( tcOutSizeID ); }
	void SetHeight( double dHeight ) { m_dHeight = dHeight; }
	void SetWidth( double dWidth, bool fWithConnection = true );
	void SetWeight( double dWeight ) { m_dWeight = dWeight; }
	void SetDegasserCompatible( double bDegasserCompatible ) { m_bDegasserCompatible = bDegasserCompatible; }
 	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	virtual void CrossVerifyID();
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	int m_iFunctions;					// Function				#ePRFunc_None/#ePRFunc_Soft/#ePRFunc_Desalin
	double m_dCapacity;					// Capacity				double ( l*�dH -> SI: m3*mol/m3 = mol)
	double m_dMaxFlow;					// MaxFlow				double (m3/s)
	double m_dMaxSupplyWaterPressure;	// MaxSupplyWaterP		double (pa)
	double m_dMinSupplyWaterPressure;	// MinSupplyWaterP		double (pa)
	IDPTR m_InConnectIDPtr;				// InConnectID			string ID
	IDPTR m_InSizeIDPtr;				// InSizeID				string ID
	IDPTR m_OutConnectIDPtr;			// OutConnectID			string ID
	IDPTR m_OutSizeIDPtr;				// OutSizeID			string ID
	double m_dHeight;					// Height				double (m)
	double m_dWidthWoConnection;		// WidthWoConnect		double (m)
	double m_dWidthWConnection;			// WidthWConnect		double (m)
	double m_dWeight;					// Weight				double (kg)
	double m_bDegasserCompatible;		// DegasserCompatible	0/1
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////
////// Total hardness characteristic
////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_TotalHardnessCharacteristic
//
//	CDB_TotalHardnessCharacteristic help to determine the max. hardness of the heating
//  water in relation to the power and the specific volume of the system.
//
//  Specific volume = System volume / smallest boiler power.
//
//  Remark: it's not implemented as a curve but as a two dimensional array.
//
//  Abbreviation: THL = Total Heat Load / vA = Specific Volume
//
//	new:CDB_TotalHardnessCharacteristic(ID) -> TOTALHARD_CHAR_TAB
//	{
//      Number of horizontal entries (excluding the definition of all vA ranges).
//      Number of vertical entries (It's in fact the number of vA range defined!!)
//      -1        \ -1        \ vA 1 min \ vA 1 max  \ vA 2 min \ vA 2 max  \ ...
//		THL 1 min \ THL 1 max \ �dH limit 1 for vA 1 \ �dH limit 1 for vA 2 \ ...
//		THL 2 min \ THL 2 max \ �dH limit 2 for vA 1 \ �dH limit 2 for vA 2 \ ...
//		...
//	}
//
//  Example from official documentation
//
//          Group | Total heat | Specific Volume | Specific Volume | Specific Volume |
//                |   load     |     limit 1     |     limit 2     |    limit 2      |
//                |   (kW)     |   < 20 l/kW     |   >= 20 l/kW    |   >= 50 l/kW    |
//                |            |                 |    < 50 l/kW    |                 |
//          -------------------+-----------------+-----------------+-----------------+
//          1     | < 50       |   <= 16.8 �dH   |   <= 11.2 �dH   |   <= 0.11 �dH   |
//          2     | 50-200     |   <= 11.2 �dH   |   <=  8.4 �dH   |   <= 0.11 �dH   |
//          3     | 200-600    |   <=  8.4 �dH   |   <= 0.11 �dH   |   <= 0.11 �dH   |
//          4     | > 600      |   <= 0.11 �dH   |   <= 0.11 �dH   |   <= 0.11 �dH   |
//
//  {
//      4																// Number of horizontal entries (excluding the definition of all VA ranges).
//      3																// Number of vertical entries (It's in fact the number of VA range defined!!)
//      -1.0  \ -1.0  \ 0.00 \ 20.0 \ 20.0 \ 50.0 \ 50.0 \ -1.0			// Definition of all VA ranges.
//      0.00  \ 50.0  \ 16.8 \ 11.2 \ 0.11
//      50.0  \ 200.0 \ 11.2 \ 8.4  \ 0.11
//      200.0 \ 600.0 \ 8.4  \ 0.11 \ 0.11
//      600.0 \ -1.0  \ 0.11 \ 0.11 \ 0.11
//  }
//
//  Remark: when you define the range, put -1.0 to specify an infinite value.
//          Ex: vA 3 range = 50.0 \ -1.0   for >=50 l/kW.
// 
///////////////////////////////////////////////////////////////////////////////////
class CDB_TotalHardnessCharacteristic : public CData
{
	DEFINE_DATA( CDB_TotalHardnessCharacteristic )

// Protected constructor/destructor.
protected:
	CDB_TotalHardnessCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_TotalHardnessCharacteristic();

// Public structure defines.
public:
	typedef struct _MinMax
	{
		double m_dMin;
		double m_dMax;
		struct _MinMax() { m_dMin = 0.0; m_dMax = 0.0; }
	}MinMax;

	typedef struct _AllArray
	{
		int m_iRows;
		int m_iCols;
		MinMax *m_prTotalHeatLoadIndex;
		MinMax *m_prSpecificVolumeIndex;
		double *m_pardMaxAdmissibleHardness;
		struct _AllArray () { m_iRows = 0; m_iCols = 0; m_prTotalHeatLoadIndex = NULL ; m_prSpecificVolumeIndex = NULL; m_pardMaxAdmissibleHardness = NULL; }
	}AllArray;

// Public methods.
public:
	// Specific volume = System volume / smallest boiler power.
	double GetTotalHardnessOfSystem( double dTotalHeatLoad, double dSpecificVolume );
	
	struct _AllArray *GetArray( void ) { return &m_rArray; }

	void CopyAllArray( AllArray *parDestinationAllArray );
	void ResetAllArray() { _Reset(); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _Reset( void );

// Private variables.
private:
	AllArray m_rArray;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_SafetyValveBase : base class for the safety valves.
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_SafetyValveBase(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		SetPressure				psv of the safety valve.
//		MediumNameID			Identifier of the medium type.
//		LiftingTypeID			Identifier of the lifting type.
//		TopOutHeight			Height (H) between the top of the product and the middle of the output (Sout).
//		OutBotHeight			Height (h) between the middle of the output (Sout) and the bottom of the product.
//		Width					Width (L) between input (Sin) and output (Sout).
//		Weight					Weight (m) of the product.
//		BlowTankID				Identifier of the blow tank allocation table.
//	}
//
////////////////////////////////////////////////////////////////
class CDB_BlowTankAllocation;
class CDB_SafetyValveBase : public CDB_TAProduct
{
	DEFINE_DATA( CDB_SafetyValveBase )

protected:
	CDB_SafetyValveBase( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_SafetyValveBase() {}

// Public methods.
public:
	CString GetInOutletConnectionString();
	CString GetInOutletSizeString();
	CString GetInOutletConnectionSizeString();

	// Return the application type for which the safety valve is compatible.
	ProjectType GetApplicationCompatibility() { return m_eApplicationCompatibility; }

	// Returns the set pressure of the safety valve.
	double GetSetPressure() { return m_dSetPressure; }
	
	const IDPTR &GetMediumNameIDPtr();
	CString GetMediumName();
	LPCTSTR GetMediumNameID() { return m_strMediumNameID; }
	
	const IDPTR &GetLiftingTypeIDPtr();
	CString GetLiftingType();
	LPCTSTR GetLiftingTypeID() { return m_strLiftingTypeID; }
	
	// Returns the total height (H) between the top of the product and the middle of the output (Sout).
	double GetTopOutHeight() { return m_dTopOutHeight; }

	// Returns the height (h) between the middle of the output (Sout) and the bottom of the product.
	double GetOutBotHeight() { return m_dOutBotHeight; }

	// Returns the width (L) between the middle of the input (Sin) until the end of the output (Sout).
	double GetWidth() { return m_dWidth; }

	// Returns the weight of the product.
	double GetWeight() { return m_dWeight; }

	// Returns IDPTR of the allocation table that helps to the blow tank selection.
	IDPTR GetBlowTankAllocationTableIDPtr();
	CDB_BlowTankAllocation *GetBlowTankAllocationTable();

	// Returns the corresponding blow tank.
	// TODO
	//CDB_BlowTank *GetBlowTank( void );

	virtual double GetPowerLimit( CString strHeatGeneratorTypeID = _T(""), CString strNormID = _T("") ) { return 0.0; }

	// SETTER.
	void SetApplicationCompatibility( ProjectType eApplicationCompatibility ) { m_eApplicationCompatibility = eApplicationCompatibility; }
	void SetSetPressure( double dSetPressure ) { m_dSetPressure = dSetPressure; }
	void SetMediumNameID( LPCTSTR tcMediumNameID ) {  _tcsncpy_s( m_strMediumNameID, SIZEOFINTCHAR( m_strMediumNameID ), tcMediumNameID, SIZEOFINTCHAR( m_strMediumNameID ) - 1 ); }
	void SetLiftingTypeID( LPCTSTR tcLiftingTypeID ) { _tcsncpy_s( m_strLiftingTypeID, SIZEOFINTCHAR( m_strLiftingTypeID ), tcLiftingTypeID, SIZEOFINTCHAR( m_strMediumNameID ) - 1 ); }
	void SetTopOutHeight( double dTopOutHeight ) { m_dTopOutHeight = dTopOutHeight; }
	void SetOutBotHeight( double dOutBotHeight ) { m_dOutBotHeight = dOutBotHeight; }
	void SetWidth( double dWidth ) { m_dWidth = dWidth; }
	void SetWeight( double dWeight ) { m_dWeight = dWeight; }
	void SetBlowTankAllocationTableID( LPCTSTR tcBlowTankAllocationTableID ) { m_BlowTankAllocationTableIDPtr.SetID( tcBlowTankAllocationTableID ); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	ProjectType m_eApplicationCompatibility;			// No read but deduced from the type ID.
	double m_dSetPressure;
	TCHAR m_strMediumNameID[_ID_LENGTH + 1];
	TCHAR m_strLiftingTypeID[_ID_LENGTH + 1];
	double m_dTopOutHeight;
	double m_dOutBotHeight;
	double m_dWidth;
	double m_dWeight;
	IDPTR m_BlowTankAllocationTableIDPtr;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_SafetyValveHeating : class for the safety valves in heating
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_SafetyValveHeating(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							0				isDeleted
//							1				isAvailable
//							0				isHidden
//							1				isFix
//							0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//							0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//							0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//						}
//						Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//						PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//						Tmax in �C
//						Tmin in �C
//					}
//					"Reference"				Reference to the TA Product
//					"Name"					Name of the product
//					TypeID					Identifier of the valve type 
//					FamilyID				Identifier of the family of the product
//					VersionID				Identifier of the version of the product
//					PriorityLevel			Level of priority for selection [1,10]
//					Comment					Product comment
//					ProdImgID				Product image ID
//					AccessGroupID			Identifier of the accessory group
//					PartOfaSet				Boolean indicating if the valve is part of a ActSet
//					Price					Price of the product
//				}
//				BodyMaterialID			Identifier of the body material
//				SizeID					Identifier of the corresp. valve size
//				ConnectID				Identifier of the (inlet/pipe side) connection
//				ConnTabID				Identifier of the connection table, can be NO_ID
//				Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//				CBIType					Type of the valve as used in the CBI II
//				CBISize					Size of the valve as used in the CBI II
//				CBISizeInch				Size of the valve in inch as used in the CBI II
//				CBICompatibilityIndex	CBI compatibility Index
//				CBIVersion				CBI Version
//				Dpmax					Maximum Dp of the valve
//				MeasPt					Measuring points
//				MeasurableData			Measurable data
//				InletSizeID				Identifier of the inlet size ID
//				OutletSizeID			Identifier of the outlet size ID
//			}
//		    AppCompatibility		Heating, cooling or solar.
//			SetPressure				psv of the safety valve.
//			MediumNameID			Identifier of the medium type.
//			LiftingTypeID			Identifier of the lifting type.
//			TopOutHeight			Height (H) between the top of the product and the middle of the output (Sout).
//			OutBotHeight			Height (h) between the middle of the output (Sout) and the bottom of the product.
//			Width					Width (L) between input (Sin) and output (Sout).
//			Weight					Weight (m) of the product.
//			BlowTankID				Identifier of the blow tank.
//		}
//		PowerLimitDirect		Maximum power allowed for the safety valve for directly heated system.
//		PowerLimitIndirect		Maximum power allowed for the safety valve for indirectly heated system.
//
////////////////////////////////////////////////////////////////
class CDB_SafetyValveHeating : public CDB_SafetyValveBase
{
	DEFINE_DATA( CDB_SafetyValveHeating )

protected:
	CDB_SafetyValveHeating( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_SafetyValveHeating() {}

// Public methods.
public:
	// Overrides the 'CDB_SafetyValveBase' virtual public methods.
	// Returns the maximum power allowed for the safety valve for directly or indirectly heated system.
	virtual double GetPowerLimit( CString strHeatGeneratorTypeID = _T("" ), CString strNormID = _T( "") );

	// SETTER.
	void SetPowerLimitDirect( double dPowerLimitDirect ) { m_dPowerLimitDirect = dPowerLimitDirect; }
	void SetPowerLimitIndirect( double dPowerLimitIndirect ) { m_dPowerLimitIndirect = dPowerLimitIndirect; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dPowerLimitDirect;
	double m_dPowerLimitIndirect;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_SafetyValveCooling : class for the safety valves in cooling
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_SafetyValveCooling(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							0				isDeleted
//							1				isAvailable
//							0				isHidden
//							1				isFix
//							0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//							0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//							0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//						}
//						Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//						PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//						Tmax in �C
//						Tmin in �C
//					}
//					"Reference"				Reference to the TA Product
//					"Name"					Name of the product
//					TypeID					Identifier of the valve type 
//					FamilyID				Identifier of the family of the product
//					VersionID				Identifier of the version of the product
//					PriorityLevel			Level of priority for selection [1,10]
//					Comment					Product comment
//					ProdImgID				Product image ID
//					AccessGroupID			Identifier of the accessory group
//					PartOfaSet				Boolean indicating if the valve is part of a ActSet
//					Price					Price of the product
//				}
//				BodyMaterialID			Identifier of the body material
//				SizeID					Identifier of the corresp. valve size
//				ConnectID				Identifier of the (inlet/pipe side) connection
//				ConnTabID				Identifier of the connection table, can be NO_ID
//				Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//				CBIType					Type of the valve as used in the CBI II
//				CBISize					Size of the valve as used in the CBI II
//				CBISizeInch				Size of the valve in inch as used in the CBI II
//				CBICompatibilityIndex	CBI compatibility Index
//				CBIVersion				CBI Version
//				Dpmax					Maximum Dp of the valve
//				MeasPt					Measuring points
//				MeasurableData			Measurable data
//				InletSizeID				Identifier of the inlet size ID
//				OutletSizeID			Identifier of the outlet size ID
//			}
//		    AppCompatibility		Heating, cooling or solar.
//			SetPressure				psv of the safety valve.
//			MediumNameID			Identifier of the medium type.
//			LiftingTypeID			Identifier of the lifting type.
//			TopOutHeight			Height (H) between the top of the product and the middle of the output (Sout).
//			OutBotHeight			Height (h) between the middle of the output (Sout) and the bottom of the product.
//			Width					Width (L) between input (Sin) and output (Sout).
//			Weight					Weight (m) of the product.
//			BlowTankID				Identifier of the blow tank.
//		}
//		PowerLimitEN12828		Maximum power allowed for the safety valve for EN12828 norm.
//		PowerLimitNone			Maximum power allowed for the safety valve without norm.
//
////////////////////////////////////////////////////////////////
class CDB_SafetyValveCooling : public CDB_SafetyValveBase
{
	DEFINE_DATA( CDB_SafetyValveCooling )

protected:
	CDB_SafetyValveCooling( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_SafetyValveCooling() {}

// Public methods.
public:
	// Overrides the 'CDB_SafetyValveBase' virtual public methods.
	// Returns the maximum power allowed for the safety valve corresponding to the 'eNorm'.
	virtual double GetPowerLimit( CString strHeatGeneratorTypeID = _T("" ), CString strNormID = _T( "") );

	// SETTER.
	void SetPowerLimitEN12828( double dPowerLimitEN12828 ) { m_dPowerLimitEN12828 = dPowerLimitEN12828; }
	void SetPowerLimitNone( double dPowerLimitNone ) { m_dPowerLimitNone = dPowerLimitNone; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dPowerLimitEN12828;
	double m_dPowerLimitNone;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_SafetyValveSolar : class for the safety valves in solar
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_SafetyValveSolar(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							0				isDeleted
//							1				isAvailable
//							0				isHidden
//							1				isFix
//							0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//							0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//							0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//						}
//						Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//						PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//						Tmax in �C
//						Tmin in �C
//					}
//					"Reference"				Reference to the TA Product
//					"Name"					Name of the product
//					TypeID					Identifier of the valve type 
//					FamilyID				Identifier of the family of the product
//					VersionID				Identifier of the version of the product
//					PriorityLevel			Level of priority for selection [1,10]
//					Comment					Product comment
//					ProdImgID				Product image ID
//					AccessGroupID			Identifier of the accessory group
//					PartOfaSet				Boolean indicating if the valve is part of a ActSet
//					Price					Price of the product
//				}
//				BodyMaterialID			Identifier of the body material
//				SizeID					Identifier of the corresp. valve size
//				ConnectID				Identifier of the (inlet/pipe side) connection
//				ConnTabID				Identifier of the connection table, can be NO_ID
//				Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//				CBIType					Type of the valve as used in the CBI II
//				CBISize					Size of the valve as used in the CBI II
//				CBISizeInch				Size of the valve in inch as used in the CBI II
//				CBICompatibilityIndex	CBI compatibility Index
//				CBIVersion				CBI Version
//				Dpmax					Maximum Dp of the valve
//				MeasPt					Measuring points
//				MeasurableData			Measurable data
//				InletSizeID				Identifier of the inlet size ID
//				OutletSizeID			Identifier of the outlet size ID
//			}
//		    AppCompatibility		Heating, cooling or solar.
//			SetPressure				psv of the safety valve.
//			MediumNameID			Identifier of the medium type.
//			LiftingTypeID			Identifier of the lifting type.
//			TopOutHeight			Height (H) between the top of the product and the middle of the output (Sout).
//			OutBotHeight			Height (h) between the middle of the output (Sout) and the bottom of the product.
//			Width					Width (L) between input (Sin) and output (Sout).
//			Weight					Weight (m) of the product.
//			BlowTankID				Identifier of the blow tank.
//		}
//		PowerLimit				Maximum power of the system allowed for the safety valve.
//		CollectorLimit			Maximum surface of collector allowed for the safety valve.
//
////////////////////////////////////////////////////////////////
class CDB_SafetyValveSolar : public CDB_SafetyValveBase
{
	DEFINE_DATA( CDB_SafetyValveSolar )

protected:
	CDB_SafetyValveSolar( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_SafetyValveSolar() {}

// Public methods.
public:
	// Returns the maximum surface of collector allowed for the safety valve.
	double GetCollectorLimit() { return m_dCollectorLimit; }

	// Overrides the 'CDB_SafetyValveBase' virtual public methods.
	// Returns the maximum power of the system allowed for the safety valve.
	virtual double GetPowerLimit( CString strHeatGeneratorTypeID = _T("" ), CString strNormID = _T( "") ) { return m_dPowerLimit; }

	// SETTER.
	void SetPowerLimit( double dPowerLimit ) { m_dPowerLimit = dPowerLimit; }
	void SetCollectorLimit( double dCollectorLimit ) { m_dCollectorLimit = dCollectorLimit; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dPowerLimit;
	double m_dCollectorLimit;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_BlowTank : Class for the blow tank ET
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_BlowTank(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						0							isDeleted
//						1							isAvailable
//						0							isHidden
//						1							isFix
//						0							Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0							Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0							Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax			in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID					Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"					Reference to the TA Product
//				"Name"						Name of the product
//				TypeID						Identifier of the valve type 
//				FamilyID					Identifier of the family of the product
//				VersionID					Identifier of the version of the product
//				PriorityLevel				Level of priority for selection [1,10]
//				Comment						Product comment
//				ProdImgID					Product image ID
//				AccessGroupID				Identifier of the accessory group
//				PartOfaSet					Boolean indicating if the valve is part of a ActSet
//				Price						Price of the product
//			}
//			BodyMaterialID				Identifier of the body material
//			SizeID						Identifier of the corresp. valve size
//			ConnectID					Identifier of the (inlet/pipe side) connection
//			ConnTabID					Identifier of the connection table, can be NO_ID
//			Conn2ID						Identifier of the outlet/body side connection, can be NO_ID
//			CBIType						Type of the valve as used in the CBI II
//			CBISize						Size of the valve as used in the CBI II
//			CBISizeInch					Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex		CBI compatibility Index
//			CBIVersion					CBI Version
//			Dpmax						Maximum Dp of the valve
//			MeasPt						Measuring points
//			MeasurableData				Measurable data
//			InletSizeID					Identifier of the inlet size ID
//			OutletSizeID				Identifier of the outlet size ID
//		}
//		ApplicationCompatibility	Type of the application (Heating, cooling or solar).
//		MediumNameID				Identifier of the medium type.
//		SteamLineSizeID				Identifier of the steam line (Sout) output size.
//		DrainLineSizeID				Identifier of the drain line (Sw) output size.
//		Weight						Weight (m) of the product.
//		Diameter					Diameter (D) of the device.
//		Height						Height (H) of the device.
//	}
//
////////////////////////////////////////////////////////////////
class CDB_BlowTank : public CDB_TAProduct
{
	DEFINE_DATA( CDB_BlowTank )

protected:
	CDB_BlowTank( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_BlowTank() {}

// Public methods.
public:
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.

	// Return the application type for which the safety valve is compatible.
	ProjectType GetApplicationCompatibility() { return m_eApplicationCompatibility; }

	const IDPTR &GetMediumNameIDPtr();
	CString GetMediumName();
	CString GetMediumNameID() { return m_tcMediumNameID; }

	// Returns the steam line (Sout) size IDPtr.
	const IDPTR &GetSteamLineSizeIDPtr();
	CString GetSteamLineSizeID();
	CString GetSteamLineSizeString( void );

	// Returns the drain line (Sw) size IDPtr.
	const IDPTR &GetDrainLineSizeIDPtr();
	CString GetDrainLineSizeID();
	CString GetDrainLineSizeString( void );

	// Returns first the 'Sin' size (connection to safety valve), followed by Sout (steam line) size and Sw (drain line) size.
	CString GetAllSizesString();

	double GetWeight( void ) { return m_dWeight; }
	double GetDiameter( void ) { return m_dDiameter; }
	double GetHeight( void ) { return m_dHeight; }
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetApplicationCompatibility( ProjectType eApplicationCompatibility ) { m_eApplicationCompatibility = eApplicationCompatibility; }
	void SetMediumNameID( _string tcMediumNameID ) { if( tcMediumNameID.compare( m_tcMediumNameID ) ) { _tcscpy_s( m_tcMediumNameID, _ID_LENGTH + 1, tcMediumNameID.c_str() ); Modified(); } }
	void SetSteamLineSizeID( _string tcSteamLineSizeID ) { m_SteamLineSizeIDPtr.SetID( tcSteamLineSizeID ); }
	void SetDrainLineSizeID( _string tcDrainLineSizeID ) { m_DrainLineSizeIDPtr.SetID( tcDrainLineSizeID ); }
	void SetWeight( double dWeight ) { m_dWeight = dWeight; }
	void SetDiameter( double dDiameter ) { m_dDiameter = dDiameter; }
	void SetHeight( double dHeight ) { m_dHeight = dHeight; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	ProjectType m_eApplicationCompatibility;			// No read but deduced from the type ID.
	TCHAR m_tcMediumNameID[_ID_LENGTH + 1];
	IDPTR m_SteamLineSizeIDPtr;
	IDPTR m_DrainLineSizeIDPtr;
	double m_dWeight;
	double m_dDiameter;
	double m_dHeight;
};
#endif	// TACBX


#ifndef TACBX
// HYS-1741
////////////////////////////////////////////////////////////////
//
//	CDB_PWQPressureReducer : Class for the Pressure reducing valve
//
////////////////////////////////////////////////////////////////
// 
//	new:CDB_PWQPressureReducer(ID)->SAFETYVALVE_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Integer to exclude product for selection: 1 (Individual/Batch), 2 (Direct) or 4 (HMCalc).
//  	   				0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				TableSetID				Table ID where are sets if exist for the current product.
//				Price					Price of the product
// 				ProductUseFlag			#eProduct, #eAccessory, #eBoth Integer to indicate if the Product is only Product, only Accessory or both
//				AccessorySeriesID		Identifier of the series of the accessory
//				isAttached				0/1: 1 if accessory cannot be ordered separately		
//  	 		AccTypeID				Identifier of Accessory type if #eBoth 
//				AccFamilyID				Identifier of Accessory family if #eBoth
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//  	MaxInletPressure        Max. inlet pressure 
//  	MinOutletPressure       Min. outlet pressure 
//  	MaxOutletPressure       Max. outlet pressure 
//	}
// Inherited Fields
//	CDB_Thing::Deleted
//	CDB_Thing::Available
//	CDB_Thing::Hidden
//	CDB_Thing::Fixed
//	CDB_Thing::FreeInt1
//	CDB_Thing::FreeInt2
//	CDB_Thing::SortInt
//	CDB_HydroThing::Pmax
//	CDB_HydroThing::PNRange
//	CDB_HydroThing::Tmax
//	CDB_HydroThing::Tmin
//	CDB_Product::Reference
//	CDB_Product::Name
//  CDB_Product::TypeID
//	CDB_Product::FamilyID
//	CDB_Product::VersionID
//	CDB_Product::PriorityLevel
//	CDB_Product::Comment
//	CDB_Product::ProdImgID
//	CDB_Product::AccessGroupID
//	CDB_Product::PartOfaSet
//  CDB_Product::TableSetID
//	CDB_Product::Price
//	CDB_Product::ProductUseFlag
//	CDB_Product::AccessorySeriesID
//	CDB_Product::isAttached
//	CDB_Product::AccTypeID
//	CDB_Product::AccFamilyID
//	CDB_TAProduct::BodyMaterialID
//	CDB_TAProduct::SizeID
//	CDB_TAProduct::ConnectID
//	CDB_TAProduct::ConnTabID
//	CDB_TAProduct::Conn2ID
//	CDB_TAProduct::CBIType
//	CDB_TAProduct::CBISize
//	CDB_TAProduct::CBISizeInch
//  CDB_TAProduct::CBICompatibilityIndex
//	CDB_TAProduct::CBIVersion
//	CDB_TAProduct::Dpmax
//	CDB_TAProduct::MeasPt
//	CDB_TAProduct::MeasurableData
//	CDB_TAProduct::InletSizeID
//	CDB_TAProduct::OutletSizeID
//	CDB_PWQPressureReducer::MaxInletPressure
//	CDB_PWQPressureReducer::MinIOutletPressure
//	CDB_PWQPressureReducer::MaxOutletPressure
//
////////////////////////////////////////////////////////////////
class CDB_PWQPressureReducer : public CDB_TAProduct
{
	DEFINE_DATA( CDB_PWQPressureReducer )

protected:
	CDB_PWQPressureReducer( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_PWQPressureReducer() {}

// Public methods.
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// GETTER.
	double GetMaxInletPressure() { return m_dMaxInletPressure; }
	double GetMinOutletPressure() { return m_dMinOutletPressure; }
	double GetMaxOutletPressure() { return m_dMaxOutletPressure; }
	CString GetOutletPressureRange();
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SETTER.
	void SetMaxInletPressure( double dMaxInletPressure ) { m_dMaxInletPressure = dMaxInletPressure; }
	void SetMinOutletPressure( double dMinOutletPressure ) { m_dMinOutletPressure = dMinOutletPressure; }
	void SetMaxOutletPressure( double dMaxOutletPressure ) { m_dMaxOutletPressure = dMaxOutletPressure; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dMaxInletPressure;
	double m_dMinOutletPressure;
	double m_dMaxOutletPressure;
};
#endif	// TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Component
//
//	new:CDB_Component(ID)->tableID	Common CData syntax
//  {
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		IDext	Identifier extension to append to the main body ID
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_Component : public CDB_TAProduct
{
	DEFINE_DATA( CDB_Component )

protected:
	CDB_Component( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Component() {}

// Public methods.
public:
	LPCTSTR GetIDExt() const { ASSERT (*m_IDext != '\0'); return m_IDext; }
	virtual LPCTSTR GetArtNum() { return GetBodyArtNum(); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	void SetIDext( LPCTSTR pID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	TCHAR m_IDext[_ID_LENGTH + 1];		// Identifier extension to append to the main body ID
};		


////////////////////////////////////////////////////////////////
//
//		CDB_RegulatingValve : Regulating valves
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_RegulatingValve(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		CharactID		Identifier of the valve characteristic
//	}
//
////////////////////////////////////////////////////////////////
class CDB_RegulatingValve : public CDB_TAProduct
{
	DEFINE_DATA( CDB_RegulatingValve )

protected:
	CDB_RegulatingValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_RegulatingValve() {}

// Public methods.
public:
	// Return the Kvs value or -1.0 if error or no Kv curve is defined for this valve.
	virtual double GetKvs();
	
	// Determine if the regulating valve is a valid control valve according to function parameters.
	bool IsValidCV( CDB_ControlProperties::CV2W3W cv2w3w, CDB_ControlProperties::eCVFUNC LastCVFUNC );
	bool IsValidCV( CDB_ControlProperties::CV2W3W cv2w3w, CDB_ControlProperties::eCVFUNC CVFunc, CDB_ControlProperties::eCTRLCHAR ctrlchar );
	
	// Retrieve the fully qualified pointer to the valve characteristic.
	//
	// Remark: Why virtual? 'CDB_ThermostaticValve' class that is inherited from 'CDB_ControlValve' (and then 'CDB_RegulatingValve'), we have 
	//         a thermostatic characteristic ('CDB_ThermostaticValve::m_pThermoCharacteristic'). But by definition (see Tadb_Trv.txt) its 'CDB_RegulatingValve' base 
	//         class does not contain any curve (NO_ID and 'CDB_RegulatingValve::m_pValveCharacteristic' will be set to NULL). 
	//         'CDB_ThermostaticValve::m_pThermoCharacteristic' contains Opening/Kvs curve in its base class ('CDB_ValveCharacteristic::m_pKvCurve') and eventually 
	//         some Opening/Kv at DT Kelvin curves in its variable member ('CDB_ThermoCharacteristic::m_pKvDTCurve').
	//		   Thus, if we want a CDB_ValveCharacteristic by calling 'CDB_RegulatingValve::GetValveCharacteristic()', it's the 'CDB_ThermostaticValve' inherited class that
	//         must return 'CDB_ThermoCharacteristic::m_pThermoCharacteristic'.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic( void );

	// Return valve characteristic data* if exist; used mainly to accelerate comparaison between char in TAP filling list
	virtual CData *GetValveCharDataPointer( void )  { return GetValveCharacteristic(); }
	
	// Retrieve the fully qualified pointer to the control properties
	virtual CDB_ControlProperties *GetCtrlProp() { return NULL; }

	virtual bool IsaCV() { return false; }
	virtual bool IsaBCV() { return false; }
	virtual bool IsaPICV() { return false; }
	virtual bool IsaTrv() { return false; }
	virtual bool IsaFLCV() { return false; }
	virtual bool IsaDPCBCV() { return false; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected members.
protected:
	void SetValveCharacteristicID( LPCTSTR pID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	TCHAR m_ValveCharacteristicID[_ID_LENGTH + 1];			// Valve characteristic ID.
	CDB_ValveCharacteristic* m_pValveCharacteristic;		// Pointer on valve characteristic.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_FixedOrifice
//
//	This is a fixed orifice.
//
//	new:CDB_FixedOrifice(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		CharactID		Identifier of the fixed office characteristic
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_FixedOrifice : public CDB_TAProduct
{
	DEFINE_DATA( CDB_FixedOrifice )
		
protected:
	CDB_FixedOrifice( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FixedOrifice() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the fixed orifice characteristic.
	CDB_FixOCharacteristic *GetFixedOrificeCharacteristic();
	
	// Returns the Kv for pressure drop value or -1.0 if no fixed orifice characteristic curve for this valve.
	double GetKv();
	
	// Returns the Kv for signal value or -1.0 if no fixed orifice characteristic curve for this valve.
	virtual double GetKvSignal();
	
	virtual bool IsKvSignalEquipped( void ) { return true; }
	
	// Virtual function has defined into CDB_TAProduct that return access to the valve characteristic.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic( void ) { return NULL; }
	
	// Return fixed orifice characteristic data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	virtual CData *GetValveCharDataPointer( void ) { return GetFixedOrificeCharacteristic(); }

	void SetCharID( _string ID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private :
	TCHAR m_FixedOrificeCharacteristicID[_ID_LENGTH + 1];		// Reference to the fixed orifice characteristic.
	CDB_FixOCharacteristic* m_pFixedOrificeCharacteristic;		// Pointer to the fixed orifice characteristic.
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_CommissioningSet
//
//	This is a commissioning set.
//
//	new:CDB_CommissioningSet(ID)->REGVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		FixOCharID		Identifier of the fixed office part characteristic
//		DrvCurveID		Identifier of the regulating valve part characteristic
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_CommissioningSet : public CDB_TAProduct
{
	DEFINE_DATA( CDB_CommissioningSet )
		
protected:
	CDB_CommissioningSet( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_CommissioningSet() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the DRV curve.
	CDB_ValveCharacteristic *GetDrvCurve();
	
	// Virtual function as defined into 'CDB_TAProduct' that returns access to the valve characteristic.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic( void ){ return GetDrvCurve(); }
	
	// Return valve characteristic data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	virtual CData *GetValveCharDataPointer( void ) { return GetDrvCurve(); }
	
	// Return the Kv for signal calculation or -1.0 if there is no fixed orifice characteristic for this valve.
	virtual double GetKvSignal();
	
	// Return the Kv for FixO Dp calculation or -1.0 if there is no fixed orifice characteristic for this valve.
	double GetKvFixO();
	
	// Return the DRV Kvs for pressure drop calculation or -1.0 if there is no fixed orifice characteristic or double regulating
	// characteristic for this valve.
	double GetKvsDrv();

	virtual bool IsKvSignalEquipped( void ) { return true; }

	void SetCharID( _string ID );
	void SetCurvID( _string ID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	// Retrieve the fully qualified pointer to the fixed orifice characteristic.
	CDB_FixOCharacteristic *GetFixedOrificeCharacteristic();
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private :
	TCHAR m_FixOCharID[_ID_LENGTH + 1];		// Reference to the fixed orifice characteristic.
	CDB_FixOCharacteristic *m_pFixOChar;	// Pointer to the fixed orifice characteristic.
	TCHAR m_DrvCurveID[_ID_LENGTH + 1];		// Reference to the double regulating valve characteristic.
	CDB_ValveCharacteristic *m_pDRVCurve;	// Pointer to the DRV curve.
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_VenturiValve
//
//	This is a Venturi valve.
//
//	new:CDB_VenturiValve(ID)->tableID	Common CData syntax
//  {
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		VenturiCharID				Venturi characteristic ID
//		VvCurveID					Venturi valve curve ID
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_VenturiValve : public CDB_TAProduct
{
	DEFINE_DATA( CDB_VenturiValve )
		
protected:
	CDB_VenturiValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_VenturiValve() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the venturi valve characteristic.
	CDB_ValveCharacteristic *GetVvCurve();

	// Virtual function has defined into CDB_TAProduct that return access to the venturi valve characteristic.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic( void ) { return GetVvCurve(); }

	// Return venturi valve characteristic data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	virtual CData *GetValveCharDataPointer(void) { return GetVvCurve(); }
	
	// Return the Kv for signal calculation or -1.0 if error or no venturi valve characteristic.
	virtual double GetKvSignal();
	
	// Return the venturi valve Kvs for pressure drop calculation or -1.0 if error or no venturi valve characteristic.
	double GetKvsVv();
	
	virtual bool IsKvSignalEquipped( void ) { return true; }

	void SetCharID( _string ID );
	void SetCurvID( _string ID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	// Retrieve the fully qualified pointer to the venturi characteristic.
	CDB_VenturiCharacteristic *GetVenturiCharacteristic();

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData(CString *pStr);
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private :
	TCHAR m_VenturiCharID[_ID_LENGTH + 1];			// Reference to the venturi characteristic.
	CDB_VenturiCharacteristic *m_pVenturiChar;		// Pointer to the venturi characteristic.
	TCHAR m_VvCurveID[_ID_LENGTH + 1];				// Reference to the venturi valve characteristic.
	CDB_ValveCharacteristic *m_pVVCurve;			// Pointer to the venturi valve characteristic.
};		


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_ShutoffValve
//
//	This is a Shut-off valve.
//
//	new:CDB_ShutoffValve(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		CharactID				Identifier of the shut-off valve characteristic
//		ActuatorGroupID			Identifier of the actuator group
//	}
///////////////////////////////////////////////////////////////////////////////////
class CDB_ShutoffValve : public CDB_TAProduct
{
	DEFINE_DATA( CDB_ShutoffValve )
		
protected:
	CDB_ShutoffValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ShutoffValve() {}

// Public methods.
public:
	void SetValveCurveID( LPCTSTR pID );
	
	// Return the Kvs value or -1.0 if error or no characteristic curve is defined for this valve.
	double GetKvs();
	
	// Return Dp for a Q or -1.0 if error or no characteristic curve is defined for this valve.
	double GetDp( double dQ, double dRho );
	
	// Virtual function has defined into CDB_TAProduct that return access to the Valve Curve
	virtual CDB_ValveCharacteristic *GetValveCharacteristic( void );

	// Return Valve Char data* if exist; used mainly to accelerate comparaison between char in TAP filling list
	virtual CData *GetValveCharDataPointer( void ) { return GetValveCharacteristic(); }

	// Retrieve ID on actuator group (override CDB_TAProduct).
	virtual IDPTR GetActuatorGroupIDPtr( void );
	void SetActuatorGroupID( _string tcActuatorGroupID ) { m_ActuatorGroupIDPtr.SetID( tcActuatorGroupID ); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	TCHAR m_ValveCurveID[_ID_LENGTH + 1];			// Reference to the valve curve.
	CDB_ValveCharacteristic *m_pCurve;				// Pointer to the curve.
	IDPTR  m_ActuatorGroupIDPtr;					// Identifier of the actuator group ID.
};


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DpController
//
//	This is a Dp controller.
//
//	new:CDB_DpController(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		CharactID	Identifier of the DpC characteristic
//		DpCLoc		Inlet pipe /OutLet pipe	
//	}
///////////////////////////////////////////////////////////////////////////////////
class CDB_DpController : public CDB_TAProduct
{
	DEFINE_DATA( CDB_DpController )

protected:
	CDB_DpController( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DpController() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the DpC characteristic.
	CDB_DpCCharacteristic *GetDpCCharacteristic();
	
	// Return stabilized value corrected with proportional band.
	double GetDpStabCorrectedWithPBfactor( double dDpStab );

	double EvaluateDpStabWithoutPBfactor(double dDpStabCorrectedWithPBFactor);
	// Returns the Kvs or -1.0 if error or no DpC characteristic curve is defined for this valve.
	double GetKvmin();
	
	// Returns the Kvm or -1.0 if error or no DpC characteristic curve is defined for this valve.
	double GetKvm();
	
	// Returns the min SP or -1.0 if error or no DpC characteristic curve is defined for this valve.
	double GetDplmin();
	
	// Returns the max SP or -1.0 if error or no DpC characteristic curve is defined for this valve.
	double GetDplmax();

	// Returns 'DpCLocDownStream' or 'DpCLocUpStream'.
	eDpCLoc	GetDpCLoc() { return m_DpCLoc; }

	// Returns a formated string with Dpl range or _T("") if error or no DpC characteristic curve is defined for this valve.
	_string GetFormatedDplRange( bool bWithUnit = true );

	virtual void GetNameEx( _string *pStr, bool bWithUnit = true );

	// Return valve characteristic data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	virtual CData *GetValveCharDataPointer( void ) { return GetDpCCharacteristic(); }

	void SetCharID( _string ID );
	void SetDpCLoc( eDpCLoc DpCLoc ) { m_DpCLoc = DpCLoc; }
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private :
	TCHAR m_DpCCharacteristicID[_ID_LENGTH + 1];		// Reference to the DpC characteristic.
	CDB_DpCCharacteristic *m_pDpCCharacteristic;						// Pointer to the DpC curve.
	eDpCLoc	m_DpCLoc;													// 'DpCLocDownStream' or 'DpCLocUpStream'.
};


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_DpReliefValve
//
//	This is a Dp relief valve.
//
//	new:CDB_DpReliefValve(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//						0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the TA Product
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				Price					Price of the product
//			}
//			BodyMaterialID			Identifier of the body material
//			SizeID					Identifier of the corresp. valve size
//			ConnectID				Identifier of the (inlet/pipe side) connection
//			ConnTabID				Identifier of the connection table, can be NO_ID
//			Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//			CBIType					Type of the valve as used in the CBI II
//			CBISize					Size of the valve as used in the CBI II
//			CBISizeInch				Size of the valve in inch as used in the CBI II
//			CBICompatibilityIndex	CBI compatibility Index
//			CBIVersion				CBI Version
//			Dpmax					Maximum Dp of the valve
//			MeasPt					Measuring points
//			MeasurableData			Measurable data
//			InletSizeID				Identifier of the inlet size ID
//			OutletSizeID			Identifier of the outlet size ID
//		}
//		MinSettingDp			Minimum preset differential pressure.
//		MaxSettingDp			Maximum preset differential pressure.
//	}
///////////////////////////////////////////////////////////////////////////////////

class CDB_DpReliefValve : public CDB_TAProduct
{
	DEFINE_DATA( CDB_DpReliefValve )

protected:
	CDB_DpReliefValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DpReliefValve() {}

// Public methods.
public:
	virtual void GetNameEx( _string *pStr, bool bWithUnit = true );

	CString GetAdjustableRange( bool bWithUnit = true );
	double GetMinSettingDp() { return m_dMinSettingDp; }
	double GetMaxSettingDp() { return m_dMaxSettingDp; }

	void SetMinSettingDp( double dMinSettingDp ) { m_dMinSettingDp = dMinSettingDp; }
	void SetMaxSettingDp( double dMaxSettingDp ) { m_dMaxSettingDp = dMaxSettingDp; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	double m_dMinSettingDp;
	double m_dMaxSettingDp;
};
#endif	// TACBX


////////////////////////////////////////////////////////////////
//
//		CDB_ControlValve : Control valves
//
//	new:CDB_ControlValve(ID)->BALCTRLVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//					"Reference"				Reference to the TA Product
//					"Name"					Name of the product
//					TypeID					Identifier of the valve type 
//					FamilyID				Identifier of the family of the product
//					VersionID				Identifier of the version of the product
//					PriorityLevel			Level of priority for selection [1,10]
//					Comment					Product comment
//					ProdImgID				Product image ID
//					AccessGroupID			Identifier of the accessory group
//					PartOfaSet				Boolean indicating if the valve is part of a ActSet
//					Price					Price of the product
//				}
//				BodyMaterialID			Identifier of the body material
//				SizeID					Identifier of the corresp. valve size
//				ConnectID				Identifier of the (inlet/pipe side) connection
//				ConnTabID				Identifier of the connection table, can be NO_ID
//				Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//				CBIType					Type of the valve as used in the CBI II
//				CBISize					Size of the valve as used in the CBI II
//				CBISizeInch				Size of the valve in inch as used in the CBI II
//				CBICompatibilityIndex	CBI compatibility Index
//				CBIVersion				CBI Version
//				Dpmax					Maximum Dp of the valve
//				MeasPt					Measuring points
//				MeasurableData			Measurable data
//				InletSizeID				Identifier of the inlet size ID
//				OutletSizeID			Identifier of the outlet size ID
//			}
//			CharactID		Identifier of the valve characteristic
//		}
//		Kvs					Kvs of the CV
//		Stroke				Stroke of the CV
//		Rangeability		Rangeability of the CV, introduce as a double or as a string ">100" only '>' is supported
//		LeakageRate			Leakage rate of the CV as a fraction of Kvs
//		CloseOffCharID		Identifier of the close-off characteristic
//		ActuatorGroupID		Identifier of the actuator group
//		CvAdapterGroupID	Identifier of the CV adapter group
//		CtrlPropID      	Identifier of the control properties
//		AdapterListMode		Set what is the method to use to retrieve adapter list linked to this control valve (#eCVAdapterListNormal or #eCVAdapterListSpecial).
//							(see enum in 'tadb.txt' for more description).
//	}
//
////////////////////////////////////////////////////////////////
#ifndef TACBX 
	class CDB_Actuator;
#endif

class CTableSet;
class CDB_ControlValve : public CDB_RegulatingValve
{
	DEFINE_DATA( CDB_ControlValve )

// Constructor and destructor.
protected:
	CDB_ControlValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ControlValve() {}

// Public enumeration defines.
public:
	enum ControlValveTable
	{
		Undefined	= -1,
		CV			= 0,
		BCV			= 1,
		PICV		= 2,
		TRV			= 3,
		FLCV		= 4,
		DPCBCV		= 5,		// i.e: TA-COMPACT-DP
	};

	// HYS-1079: Fail-safe function is redefined and now we use default return position 
	enum DRPFunction
	{
		drpfUndefined = -1,
		drpfNone,
		drpfClosing,
		drpfOpening,
		drpfOpeningOrClosing,
		drpfAll,
		drpfLast
	};
	
	enum AdapterListMode
	{
		almNotApplicable = -1,
		almNormal,
		almSpecial
	};
	
// Public methods.
public:

	double GetKvs();
	double GetStroke() { return m_dStroke; }
	double GetRangeability() { return m_dRangeability; }
	_string GetStrRangeability() { if ( m_dRangeability <= 0 ) return _string(L"-"); else return m_strRangeability; }
	double GetLeakageRate() { return m_dLeakageRate; }
	IDPTR  GetCloseOffCharIDPtr();

	// Retrieve ID on actuator group (override CDB_TAProduct).
	virtual IDPTR GetActuatorGroupIDPtr( void );
	
	// Retrieve ID on adapter group (override CDB_TAProduct).
	virtual IDPTR GetAdapterGroupIDPtr( void );
	
	IDPTR GetCtrlPropIDPtr();
	virtual CDB_ControlProperties *GetCtrlProp() { return (CDB_ControlProperties *)GetCtrlPropIDPtr().MP; }
	
	AdapterListMode GetAdapterListMode( void ) { return m_eAdapterListMode; }

	virtual bool IsaCV() { return ( CDB_ControlProperties::ControlOnly == GetCtrlProp()->GetCvFunc() ); }
	virtual bool IsaBCV() { return 	( CDB_ControlProperties::ControlOnly != GetCtrlProp()->GetCvFunc() ); }

#ifndef TACBX 
	int GetCharacteristicImageID();
	int GetPushPullCloseImageID();
	CString GetStrokeString();

	// Allow to retrieve what is the default return position function compatible for the current control valve and the actuator.
	// Remark: for compilation problem, we use here the 'int' type instead of 'CDB_ElectroActuator::DefaultReturnPosition'.
	DRPFunction GetCompatibleDRPFunction( int iDefaultReturnPosition );

	// Allow to retrieve what is the actuator default return position function mode compatible with the current control valve.
	// Remark: for compilation problem, we return here the 'int' type instead of 'CDB_ElectroActuator:DefaultReturnPosition'.
	int GetCompatibleActuatorDRP( CDB_ControlValve::DRPFunction DRPFunction );

	ControlValveTable GetCVParentTable( void );
	
	// Retrieve a pointer on table containing actuator set.
	virtual CTableSet *GetTableSet( );
	
	// Verify if an actuator could be used with the current control valve (Scan the actuator group).
	bool IsActuatorFit( CDB_Actuator *pActr );
	
	// Return adapter that matches a control valve and an actuator.
	CDB_Product *GetMatchingAdapter( CDB_Actuator *pclActuator, bool bOnlyForSet = false );
	
	// Fill a list with actuator that are compliant with the defined package
	int ExtractPackageCompliantActr ( std::multimap< double, CDB_Actuator *> *pOrgActrList, std::multimap< double, CDB_Actuator *> *pPackageCompliantList );

#endif //TACBX
	void SetKvs( double dKvs ) { if( dKvs != m_dKvs ) { m_dKvs = dKvs; Modified(); } }
	void SetStroke( double dStroke ){ if( dStroke != m_dStroke ) { m_dStroke = dStroke; Modified(); } }
	void SetRangeability( double dRangeability ) { if( dRangeability != m_dRangeability ) { m_dRangeability = dRangeability; Modified(); } }
	void SetstrRangeability( _string strRangeability ) { if ( strRangeability.compare(m_strRangeability) ) { m_strRangeability = strRangeability; Modified(); } };
	void SetLeakageRate( double dLeakageRate ) { if( dLeakageRate != m_dLeakageRate ) { m_dLeakageRate = dLeakageRate; Modified(); } }
	void SetCloseOffCharID( _string strCloseOffCharID ) { m_CloseOffCharIDPtr.SetID( strCloseOffCharID ); }
	void SetActuatorGroupID( _string strActuatorGroupID ) { m_ActuatorGroupIDPtr.SetID( strActuatorGroupID ); }
	void SetCvAdapterGroupID( _string strCvAdapterGroupID ) { m_CvAdapterGroupIDPtr.SetID( strCvAdapterGroupID ); }
	void SetCtrlPropID( _string strCtrlPropID ) { m_CtrlPropIDPtr.SetID( strCtrlPropID ); }
	void SetAdapterListMode( AdapterListMode eAdapterListMode ) { if( eAdapterListMode != m_eAdapterListMode ) { m_eAdapterListMode = eAdapterListMode; Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dKvs;
	double m_dStroke;
	double m_dRangeability;
	_string m_strRangeability;
	double m_dLeakageRate;
	IDPTR  m_CloseOffCharIDPtr;
	IDPTR  m_ActuatorGroupIDPtr;
	IDPTR  m_CvAdapterGroupIDPtr;
	IDPTR  m_CtrlPropIDPtr;
	AdapterListMode m_eAdapterListMode;
};


////////////////////////////////////////////////////////////////
//
//		CDB_PIControlValve : Pressure independent control valves
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_PIControlValve(ID)->PICTRLVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//						"Reference"				Reference to the TA Product
//						"Name"					Name of the product
//						TypeID					Identifier of the valve type 
//						FamilyID				Identifier of the family of the product
//						VersionID				Identifier of the version of the product
//						PriorityLevel			Level of priority for selection [1,10]
//						Comment					Product comment
//						ProdImgID				Product image ID
//						AccessGroupID			Identifier of the accessory group
//						PartOfaSet				Boolean indicating if the valve is part of a ActSet
//						Price					Price of the product
//					}
//					BodyMaterialID			Identifier of the body material
//					SizeID					Identifier of the corresp. valve size
//					ConnectID				Identifier of the (inlet/pipe side) connection
//					ConnTabID				Identifier of the connection table, can be NO_ID
//					Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//					CBIType					Type of the valve as used in the CBI II
//					CBISize					Size of the valve as used in the CBI II
//					CBISizeInch				Size of the valve in inch as used in the CBI II
//					CBICompatibilityIndex	CBI compatibility Index
//					CBIVersion				CBI Version
//					Dpmax					Maximum Dp of the valve
//					MeasPt					Measuring points
//					MeasurableData			Measurable data
//					InletSizeID				Identifier of the inlet size ID
//					OutletSizeID			Identifier of the outlet size ID
//				}
//				CharactID				// MUST BE SET TO "NO_ID" -> valve characteristic is set now in PICVCharID
//			}
//			Kvs						Kvs of the CV (Kvs is the Kv at full opening)
//			Stroke					Stroke of the CV
//			Rangeability			Rangeability of the CV, introduce as a double or as a string ">100" only '>' is supported
//			LeakageRate				Leakage rate of the CV as a fraction of Kvs
//			CloseOffCharID			Identifier of the close-off characteristic
//			ActuatorGroupID			Identifier of the actuator group
//			CvAdapterGroupID		Identifier of the CV adapter group
// 			CtrlPropID				Identifier of the control properties
//			AdapterListMode			Set what is the method to use to retrieve adapter list linked to this control valve (#eCVAdapterListNormal or #eCVAdapterListSpecial).
//									(see enum in 'tadb.txt' for more description).
//		}
//		PICVCharID		Identifier of the PICV characteristic
//	}
//
////////////////////////////////////////////////////////////////
class CDB_PIControlValve : public CDB_ControlValve
{
	DEFINE_DATA( CDB_PIControlValve )

protected:
	CDB_PIControlValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_PIControlValve() {}
	
// Public methods.
public:
	// Retrieve the fully qualified pointer to PICV characteristic.
	CDB_PICVCharacteristic *GetPICVCharacteristic();

	// Retrieve the fully qualified pointer to the valve characteristic define in the 'CDB_PICVCharacteristic'.
	// Remark: overriding 'CDB_TAProduct' base class.
	// Remark: for 'CDB_PIControlValve', 'CDB_RegulatingValve::CharactID' is set to 'NO_ID' because 'CDB_PICVCharacteristic' is inherited from
	//         'CDB_ValveCharacteristic'.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic() { return GetPICVCharacteristic(); }

	// Get Dpmin from the flow. TURBULENT FLOW ASSUMED with and without Fc variation effects.
	// With Fc variation effects: data and model are missing on combined viscosity and Fc variation effects.
	// Without Fc variation effects: data (and model) missing on viscosity effect in Dp control part.
	// Returns Dpmin or -1.0 if error or no PICV characteristic for this valve.
	double GetDpmin( double dFlow, double dDensity );
	
	// Returns the max setting or -1.0 if error or no PICV characteristic for this valve.
	double GetMaxSetting();
	
	// Get presetting from the flow. TURBULENT FLOW ASSUMED with Fc variation effects.
	// With Fc variation effects: data and model are missing on combined viscosity and Fc variation effects.
	// Returns the presetting or  -1.0 if error or no PICV characteristic for this valve.
	double GetPresetting( double dFlow, double dDensity, double dViscosity );
	
	// Returns Kvd or -1.0 if error or no PICV characteristic for this valve.
	double GetKvd();

	// Returns Fc or -1.0 if error or no PICV characteristic for this valve.
	double GetFc();

	// Returns Qmax or -1.0 if error or no PICV characteristic for this valve.
	double GetQmax();

	// Return Valve Char data* if exist; used mainly to accelerate comparaison between char in TAP filling list
	virtual CData *GetValveCharDataPointer( void ) { return GetPICVCharacteristic(); }
	
	virtual bool IsaBCV() { return false; }
	virtual bool IsaCV() { return false; }
	virtual bool IsaPICV() { return true; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CDB_Product' public virtual method.
#ifndef TACBX
	// Retrieve a pointer on table containing actuator set.
	virtual CTableSet *GetTableSet();
#endif
	///////////////////////////////////////////////////////////////////////////////////
	
// Protected methods.
protected:
	void SetPICVCharacteristicID( LPCTSTR strCharacteristicID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	TCHAR m_PICVCharacteristicID[_ID_LENGTH + 1];			// The PICV characteristic ID.
	CDB_PICVCharacteristic* m_pPICVCharacteristic;			// Pointer on the PICV characteristic.
};


#ifndef TACBX 
////////////////////////////////////////////////////////////////
//
//		HYS-1144: Table 6WAYCTRLVALV_TAB : 6 way ctrl valves
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_6WayValve(ID)->6WAYCTRLVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//      						0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//						"Reference"				Reference to the TA Product
//						"Name"					Name of the product
//						TypeID					Identifier of the valve type
//						FamilyID				Identifier of the family of the product
//						VersionID				Identifier of the version of the product
//						PriorityLevel			Level of priority for selection [1,10]
//						Comment					Product comment
//						ProdImgID				Product image ID
//						AccessGroupID			Identifier of the accessory group
//						PartOfaSet				Boolean indicating if product is part of a set. Can be #ePartOfaSetNo, #ePartOfaSetYes, #ePartOfaSetYesBreakOk or #ePartOfaSetYesBreakOnMsg.
//						TableSetID				Table ID where are sets if exist for the current product.
//						Price					Price of the product
// 						ProductUseFlag			#eProduct, #eAccessory, #eBoth Integer to indicate if the Product is only Product, only Accessory or both
//						AccessorySeriesID		Identifier of the series of the accessory
//						isAttached				0/1: 1 if accessory cannot be ordered separately		
//   					AccTypeID				Identifier of Accessory type if #eBoth 
//						AccFamilyID				Identifier of Accessory family if #eBoth
//					}
//					BodyMaterialID			Identifier of the body material
//					SizeID					Identifier of the corresp. valve size
//					ConnectID				Identifier of the (inlet/pipe side) connection
//					ConnTabID				Identifier of the connection table, can be NO_ID
//					Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//					CBIType					Type of the valve as used in the CBI II
//					CBISize					Size of the valve as used in the CBI II
//					CBISizeInch				Size of the valve in inch as used in the CBI II
//					CBICompatibilityIndex	CBI compatibility Index
//					CBIVersion				CBI Version
//					Dpmax					Maximum Dp of the valve
//					MeasPt					Measuring points. #eMeasPointNone or a combination of #eMeasPointInlet, #eMeasPointOutlet, #eMeasPointMeasStatOut or #eMeasPointP+.
//					MeasurableData			Measurable data. #eMeasDataNone or a combination of #eMeasDataDp, #eMeasDataFlow, #eMeasDataDpSignal, #eMeasDataDpAvail, #eMeasDataDpStab or #eMeasDataDpCAlwaysOn.
//					InletSizeID				Identifier of the inlet size ID
//					OutletSizeID			Identifier of the outlet size ID
//				}
//				CharactID				// MUST BE SET TO "NO_ID" -> valve characteristic is set now in PICVCharID
//			}
//			Kvs						Kvs of the CV (Kvs is the Kv at full opening)
//			Stroke					Stroke of the CV
//			Rangeability			Rangeability of the CV, introduce as a double or as a string ">100" only '>' is supported
//			LeakageRate				Leakage rate of the CV as a fraction of Kvs
//			CloseOffCharID			Identifier of the close-off characteristic
//			ActuatorGroupID			Identifier of the actuator group
//			CvAdapterGroupID		Identifier of the CV adapter group
// 			CtrlPropID				Identifier of the control properties
//			AdapterListMode			Set what is the method to use to retrieve adapter list linked to this control valve (#eCVAdapterListNormal or #eCVAdapterListSpecial).
//									(see enum in 'tadb.txt' for more description).
//		}
//		6WayQmax				Maximum flow rate across the valve
//		PicvGroupID				Table that contains IDs of all the PIBCV that can be used with this 6 way valve
//		BvFamiliesGroupID		Table that contains IDs of all the BV family IDs that can be used with this 6 way valve
//	}
class CDB_6WayValve : public CDB_ControlValve
{
	DEFINE_DATA( CDB_6WayValve )

protected:
	CDB_6WayValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_6WayValve() {}

// Public methods.
public:
	// Returns Qmax or -1.0 if error or no PICV characteristic for this valve.
	double GetQmax() { return m_dQmax; }
	IDPTR GetPicvGroupTableIDPtr();
	IDPTR GetBvFamiliesGroupTableIDPtr();

	virtual bool IsaBCV() { return false; }
	virtual bool IsaCV() { return true; }
	virtual bool IsaPICV() { return false; }
	virtual bool Isa6WCV() { return true; }

	virtual CTableSet* GetTableSet();
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	void SetQmax( double dNewQmax ) { if (0 < dNewQmax) m_dQmax = dNewQmax; }
	void SetPicvGroupTableIDPtr( _string ID );
	void SetBvFamiliesGroupTableIDPtr( _string ID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	// Retrieve a pointer on table containing actuator set.
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dQmax;					// The 6 way valve Qmax.
	IDPTR m_PICVGroupTabIDPtr;		// idptr on the PICV group table. 
	IDPTR m_BVFamilyGroupTabIDPtr;	// idptr on the BV families group table. 
};
#endif	// TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_SmartValveCharacteristic
//
//	CDB_SmartValveCharacteristic is characteristic defined for selection of smart valves.
//
//	new:CDB_SmartValveCharacteristic(ID\1) -> SMARTCONTROLVALVE_CHAR_TAB
//	{
//		{
//			Discrete/Continuous enum				Discrete set of data or continuous characteristic (#eDISCRETE_True, #eDISCRETE_False or #eDISCRETE_Undefined).
//			Single/Multi/Electronic-turn enum		Number of turns of the valve (#eTURN_Single, #eTURN_Multi, e#TURN_Electronic or #eTURN_Undefined).
//			SettingRounding							Allows to know how to round the result of a computing setting.
//		}
//		Kvs
//		Minimum adjustable max flow
//      Nominal flow
//      Minimum adjustable max power
//      Nominal power
//	}
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_SmartValveCharacteristic : public CDB_Characteristic
{
	DEFINE_DATA( CDB_SmartValveCharacteristic )
				
protected:
	CDB_SmartValveCharacteristic( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_SmartValveCharacteristic() {}

// Public methods.
public:
	double GetKvs() { return m_dKvs; }
	// The method returns the minimal adjustable flow as defined in the characteristic.
	// Ex: 920 l/h for TA-Smart DN 32 with a Qnom = 4600 l/h. If a unit terminal needs 700 l/h, we don't 
	// allow TA-Smart DN32 to work with it. Because below 20% of Qnom (Here 720 l/h) the control accuracy 
	// is becoming bad (Deviation > 5%).
	double GetMinAdjustableFlow( void );

	// The method returns the nominal flow as defined in the characteristic.
	double GetQnom( void );

	// The method returns the minimal adjustable power as defined in the characteristic.
	// Ex: 2,134 kW for TA-Smart DN 32 with a Pnom = 267 kW. If a unit terminal needs 2000 kW, we don't 
	// allow TA-Smart DN32 to work with it. Because the control accuracy is becoming bad (Deviation > 5%).
	double GetMinAdjustablePower();

	// The method returns the nominal power as defined in the characteristic.
	double GetPowerNom( void );
	
// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dKvs;
	double m_dMinAdjustableFlow;		// The minimum adjustable flow.
	double m_dQnom;						// The nominal flow.
	double m_dMinAdjustablePower;		// The minimum adjustable power.
	double m_dPowerNom;					// The nominal power.
};


////////////////////////////////////////////////////////////////
//
//		CDB_SmartControlValve : TA-Smart
//
//	new:CDB_SmartControlValve(ID)->SMARTCONTROLVALVE_TAB (or SMARTDIFFPRESS_TAB)
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0							isDeleted
//								1							isAvailable
//								0							isHidden
//								1							isFix
//								0							Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0							Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0							Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID					Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//					"Reference"						Reference to the TA Product
//					"Name"							Name of the product
//					TypeID							Identifier of the valve type 
//					FamilyID						Identifier of the family of the product
//					VersionID						Identifier of the version of the product
//					PriorityLevel					Level of priority for selection [1,10]
//					Comment							Product comment
//					ProdImgID						Product image ID
//					AccessGroupID					Identifier of the accessory group
//					PartOfaSet						Boolean indicating if the valve is part of a ActSet
//					Price							Price of the product
//				}
//				BodyMaterialID					Identifier of the body material
//				SizeID							Identifier of the corresp. valve size
//				ConnectID						Identifier of the (inlet/pipe side) connection
//				ConnTabID						Identifier of the connection table, can be NO_ID
//				Conn2ID							Identifier of the outlet/body side connection, can be NO_ID
//				CBIType							Type of the valve as used in the CBI II
//				CBISize							Size of the valve as used in the CBI II
//				CBISizeInch						Size of the valve in inch as used in the CBI II
//				CBICompatibilityIndex			CBI compatibility Index
//				CBIVersion						CBI Version
//				Dpmax							Maximum Dp of the valve
//				MeasPt							Measuring points
//				MeasurableData					Measurable data
//				InletSizeID						Identifier of the inlet size ID
//				OutletSizeID					Identifier of the outlet size ID
//			}
//			CharactID						Identifier of the valve characteristic
//		}
// 		SmartValveType					Can be #SmartValveTypeControl or #SmartValveTypeDpC.
//		Stroke							Stroke of the smart control valve
//		Rangeability					Rangeability of the smart control valve, introduce as a double or as a string ">100" only '>' is supported
//		LeakageRate						Leakage rate of the smart control valve as a fraction of Kvs
//		CtrlPropID      				Identifier of the control properties
//      InternalActuatorName			Internal actuator name.
//		CharactID						Identifier of the smart control valve characteristic.
//		RemoteTempSensorCableLength		Define the remote temperature sensor cable length.
//		Power supply			        List of accepted Power supplies separated by "\"
//		Input signals			        List of accepted input control signals separated by "\"
//		Output signals			        List of accepted output control signals separated by "\"
//		DpSensorGroupID					Identifier of the Dp sensor group.
//	}
//
////////////////////////////////////////////////////////////////
class CDB_SmartControlValve : public CDB_RegulatingValve
{
	DEFINE_DATA( CDB_SmartControlValve )

// Constructor and destructor.
protected:
	CDB_SmartControlValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_SmartControlValve() {}

// Public methods.
public:
	SmartValveType GetSmartValveType() { return m_eSmartValveType; }
	double GetStroke() { return m_dStroke; }
	double GetRangeability() { return m_dRangeability; }
	_string GetStrRangeability() { if ( m_dRangeability <= 0 ) return _string(L"-"); else return m_strRangeability; }
	double GetLeakageRate() { return m_dLeakageRate; }
	IDPTR GetCtrlPropIDPtr();
	virtual CDB_ControlProperties *GetCtrlProp() { return (CDB_ControlProperties *)GetCtrlPropIDPtr().MP; }
	_string GetInternalActuatorName() { return m_strInternalActuatorName; }
	CDB_SmartValveCharacteristic *GetSmartValveCharacteristic();
	double GetRemoteTempSensorCableLength() { return m_dRemoteTempSensorCableLength; }
	IDPTR GetDpSensorGroupIDPtr();

	double GetMinAdjustableFlow();
	double GetQNom();

	double GetMinAdjustablePower();
	double GetPNom();

	double GetKvs();
	double GetDpMin( double dFlow, double dRho );

#ifndef TACBX 
	CString GetStrokeString();

	// HYS-1660: Add new information for the smart control valve.
	unsigned int GetNumberOfPowerSupplyIDPtr() { return (int)m_arPowerSupplies.GetCount(); }
	IDPTR GetPowerSupplyIDPtr( unsigned int index );
	CString GetPowerSupplyStr( CString strSeparator = _T( "; " ) );

	unsigned int GetNumberOfInputSignalIDPtr() { return (int)m_arInputSignals.GetCount(); }
	IDPTR GetInputSignalIDPtr( unsigned int index );

	unsigned int GetNumberOfOutputSignalIDPtr() { return (int)m_arOutputSignals.GetCount(); }
	IDPTR GetOutputSignalIDPtr( unsigned int index );

	CString GetInOutSignalsStr( bool bInput, CString strSeparator = _T( "; " ) );
	void SetPowerSupplyIDPtr( CDB_SmartControlValve *pclCopySrc );
	void SetInputSignalIDPtr( CDB_SmartControlValve *pclCopySrc );
	void SetOutputSignalIDPtr( CDB_SmartControlValve *pclCopySrc );

	CString GetControlModeString( SmartValveControlMode eControlMode );
	CString GetLocalizationString( SmartValveLocalization eLocalization );
#endif //TACBX

	void SetSmartValveType( SmartValveType eSmartValveType ) { if( eSmartValveType != m_eSmartValveType ){ m_eSmartValveType = eSmartValveType; Modified(); } }
	void SetStroke( double dStroke ){ if( dStroke != m_dStroke ) { m_dStroke = dStroke; Modified(); } }
	void SetRangeability( double dRangeability ) { if( dRangeability != m_dRangeability ) { m_dRangeability = dRangeability; Modified(); } }
	void SetstrRangeability( _string strRangeability ) { if( strRangeability.compare(m_strRangeability) ) { m_strRangeability = strRangeability; Modified(); } };
	void SetLeakageRate( double dLeakageRate ) { if( dLeakageRate != m_dLeakageRate ) { m_dLeakageRate = dLeakageRate; Modified(); } }
	void SetCtrlPropID( _string strCtrlPropID ) { m_CtrlPropIDPtr.SetID( strCtrlPropID ); }
	void SetInternalActuatorName( _string strName ) { if( m_strInternalActuatorName != strName ) { m_strInternalActuatorName = strName; Modified(); } }
	void SetSmartValveCharacteristicID( _string ID );
	void SetRemoteTempSensorCableLength( double val ) { if( val != m_dRemoteTempSensorCableLength ) { m_dRemoteTempSensorCableLength = val; Modified(); } }
	void SetDpSensorGroupID( _string strDpSensorGropuID ) { if( strDpSensorGropuID.compare( m_DpSensorGroupIDPtr.ID ) ) { _tcscpy_s( m_DpSensorGroupIDPtr.ID, _ID_LENGTH + 1, strDpSensorGropuID.c_str() ); Modified(); } }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	SmartValveType m_eSmartValveType;
	double m_dStroke;
	double m_dRangeability;
	_string m_strRangeability;
	double m_dLeakageRate;
	IDPTR  m_CtrlPropIDPtr;
	_string m_strInternalActuatorName;
	TCHAR m_SmartValveCharacteristicID[_ID_LENGTH + 1];					// Smart valve characteristic ID.
	double m_dRemoteTempSensorCableLength;
	CDB_SmartValveCharacteristic *m_pSmartValveCharacteristic;			// Pointer on the smart valve characteristic.
	IDPTR m_DpSensorGroupIDPtr;

// HYS-1660
#ifndef TACBX
	CArray <IDPTR> m_arPowerSupplies;
	CArray <IDPTR> m_arInputSignals;
	CArray <IDPTR> m_arOutputSignals;
#endif
};


////////////////////////////////////////////////////////////////
//
//		CDB_ThermostaticValve : This is a thermostatic radiator valve.
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_ThermostaticValve(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//								Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//						"Reference"				Reference to the TA Product
//						"Name"					Name of the product
//						TypeID					Identifier of the valve type 
//						FamilyID				Identifier of the family of the product
//						VersionID				Identifier of the version of the product
//						PriorityLevel			Level of priority for selection [1,10]
//						Comment					Product comment
//						ProdImgID				Product image ID
//						AccessGroupID			Identifier of the accessory group
//						PartOfaSet				Boolean indicating if the valve is part of a ActSet
//						Price					Price of the product
//					}
//					BodyMaterialID			Identifier of the body material
//					SizeID					Identifier of the corresp. valve size
//					ConnectID				Identifier of the (inlet/pipe side) connection
//					ConnTabID				Identifier of the connection table, can be NO_ID
//					Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//					CBIType					Type of the valve as used in the CBI II
//					CBISize					Size of the valve as used in the CBI II
//					CBISizeInch				Size of the valve in inch as used in the CBI II
//					CBICompatibilityIndex	CBI compatibility Index
//					CBIVersion				CBI Version
//					Dpmax					Maximum Dp of the valve
//					MeasPt					Measuring points
//					MeasurableData			Measurable data
//					InletSizeID				Identifier of the inlet size ID
//					OutletSizeID			Identifier of the outlet size ID
//				}
//				CharactID		// MUST BE SET TO "NO_ID" -> valve characteristic is set now in ThermoCharID
//			}
//			Kvs					Kvs of the CV
//			Stroke				Stroke of the CV
//			Rangeability		Rangeability of the CV
//			LeakageRate			Leakage rate of the CV as a fraction of Kvs
//			CloseOffCharID		Identifier of the close-off characteristic
//			ActuatorGroupID		Identifier of the actuator group
//			CvAdapterGroupID	Identifier of the CV adapter group
//			CtrlPropID			Identifier of the control properties
//			AdapterListMode		Set what is the method to use to retrieve adapter list linked to this control valve (#eCVAdapterListNormal or #eCVAdapterListSpecial).
//								(see enum in 'tadb.txt' for more description).
//		}
//		ThermoCharID		Identifier of the thermostatic characteristic
//		"Description"		Description of the product
//	}
//
////////////////////////////////////////////////////////////////
class CDB_ThermostaticValve : public CDB_ControlValve
{
	DEFINE_DATA( CDB_ThermostaticValve )

protected:
	CDB_ThermostaticValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ThermostaticValve() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the valve curve.
	CDB_ThermoCharacteristic *GetThermoCharacteristic();

	// Return description of the product.
	LPCTSTR GetDescription();

	// Retrieve the fully qualified pointer to the valve characteristic define in the 'CDB_PICVCharacteristic'.
	// Remark: overriding 'CDB_TAProduct' base class.
	// Remark: for 'CDB_ThermostaticValve', 'CDB_RegulatingValve::CharactID' is set to 'NO_ID' because 'CDB_ThermoCharacteristic' is inherited from
	//         'CDB_ValveCharacteristic'.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic() { return GetThermoCharacteristic(); }

	// Return thermostatic characteristic data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	// Remark: overriding 'CDB_TAProduct' base class.
	virtual CData *GetValveCharDataPointer( void ) { return GetThermoCharacteristic(); }
	
	virtual bool IsaCV() { return false; }
	virtual bool IsaBCV() { return false; }
	virtual bool IsaTrv() { return true; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:
	void SetThermoCharacteristicID( LPCTSTR strCharacteristicID );
	LPCTSTR SetDescription( LPCTSTR strDescription );
		
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private :
	TCHAR m_ThermoCharacteristicID[_ID_LENGTH + 1];			// The thermostatic characteristic ID.
	CDB_ThermoCharacteristic* m_pThermoCharacteristic;		// Pointer on the thermostatic characteristic.
	_string m_strDescription;								// Description of the product.
	_string *m_pStr;
};


////////////////////////////////////////////////////////////////
//
//		CDB_FlowLimitedControlValve : This is a control valve for radiator that has a flow limited built-in.
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_FlowLimitedControlValve(ID)->tableID	Common CData syntax
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//						"Reference"				Reference to the TA Product
//						"Name"					Name of the product
//						TypeID					Identifier of the valve type 
//						FamilyID				Identifier of the family of the product
//						VersionID				Identifier of the version of the product
//						PriorityLevel			Level of priority for selection [1,10]
//						Comment					Product comment
//						ProdImgID				Product image ID
//						AccessGroupID			Identifier of the accessory group
//						PartOfaSet				Boolean indicating if the valve is part of a ActSet
//						Price					Price of the product
//					}
//					BodyMaterialID			Identifier of the body material
//					SizeID					Identifier of the corresp. valve size
//					ConnectID				Identifier of the (inlet/pipe side) connection
//					ConnTabID				Identifier of the connection table, can be NO_ID
//					Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//					CBIType					Type of the valve as used in the CBI II
//					CBISize					Size of the valve as used in the CBI II
//					CBISizeInch				Size of the valve in inch as used in the CBI II
//					CBICompatibilityIndex	CBI compatibility Index
//					CBIVersion				CBI Version
//					Dpmax					Maximum Dp of the valve
//					MeasPt					Measuring points
//					MeasurableData			Measurable data
//					InletSizeID				Identifier of the inlet size ID
//					OutletSizeID			Identifier of the outlet size ID
//				}
//				CharactID		// MUST BE SET TO "NO_ID" -> valve characteristic is set now in FLCVCharID
//			}
//			Kvs					Kvs of the CV
//			Stroke				Stroke of the CV
//			Rangeability		Rangeability of the CV
//			LeakageRate			Leakage rate of the CV as a fraction of Kvs
//			CloseOffCharID		Identifier of the close-off characteristic
//			ActuatorGroupID		Identifier of the actuator group
//			CvAdapterGroupID	Identifier of the CV adapter group
//			CtrlPropID			Identifier of the control properties
//			AdapterListMode		Set what is the method to use to retrieve adapter list linked to this control valve (#eCVAdapterListNormal or #eCVAdapterListSpecial).
//								(see enum in 'tadb.txt' for more description).
//		}
//		FLCVCharID			Identifier of the flow limited control valve characteristic
//		"Description"		Description of the product
//	}
//
////////////////////////////////////////////////////////////////
class CDB_FlowLimitedControlValve : public CDB_ControlValve
{
	DEFINE_DATA( CDB_FlowLimitedControlValve )

protected:
	CDB_FlowLimitedControlValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FlowLimitedControlValve() {}

// Public methods.
public:
	// Retrieve the fully qualified pointer to the flow limited control valve characteristic.
	CDB_FLCVCharacteristic *GetFLCVCharacteristic();

	// Return description of the product.
	LPCTSTR GetDescription();

	// Return flow limited control valve characteristic data* if exist; used mainly to accelerate comparaison between 
	// char in TAP filling list.
	// Remark: overriding 'CDB_TAProduct' base class.
	virtual CData *GetValveCharDataPointer(void) { return GetFLCVCharacteristic(); }

	virtual bool IsaCV() { return false; }
	virtual bool IsaBCV() { return false; }
	virtual bool IsaFLCV() { return true; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:
	void SetFLCVCharacteristicID( LPCTSTR strCharacteristicID );
	LPCTSTR SetDescription( LPCTSTR strDescription );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private :
	TCHAR m_FLCVCharacteristicID[_ID_LENGTH + 1];			// The flow limited control valve characteristic ID.
	CDB_FLCVCharacteristic *m_pFLCVCharacteristic;			// Pointer on the flow limited control valve characteristic.
	_string m_strDescription;								// Description of the product.
	_string *m_pStr;
};


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_FloorHeatingManifold : Floor heating manifolds
//
//	new:CDB_FloorHeatingManifold(ID)->FLOORHCONTROL_TAB
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//		HeatingCircuits			Define the number of heating circuits.
//	}
//
////////////////////////////////////////////////////////////////
class CDB_FloorHeatingManifold : public CDB_Product
{
	DEFINE_DATA( CDB_FloorHeatingManifold )

protected:
	CDB_FloorHeatingManifold( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FloorHeatingManifold() {}

// Public methods.
public:
	virtual void GetNameEx( _string *pStr, bool bWithUnit = true );

	int GetHeatingCircuits() { return m_iHeatingCircuits; }

	void SetHeatingCircuits( int iHeatingCircuits ) { m_iHeatingCircuits = iHeatingCircuits; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int m_iHeatingCircuits;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_FloorHeatingValve : Floor heating control valves
//
//	new:CDB_FloorHeatingValve(ID)->FLOORHCONTROL_TAB
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//		ConnectInletTypeID			Connection inlet type ID.
//		ConnectInletSizeID			Connection inlet size ID.
// 		ConnectOutletTypeID			Connection outlet type ID.
// 		ConnectOutletSizeID			Connection outlet size ID.
//		Handwheel					0 = without handweel, 1 = with handwheel
//	}
//
////////////////////////////////////////////////////////////////
class CDB_FloorHeatingValve : public CDB_Product
{
	DEFINE_DATA( CDB_FloorHeatingValve )

protected:
	CDB_FloorHeatingValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FloorHeatingValve() {}

// Public methods.
public:
	int GetSupplyOrLockshield() { return m_iSupplyOrLockshield; }
	CString GetSupplyOrLockshieldString();

	const IDPTR &GetConnectInletTypeIDPtr();
	CString GetConnectInletTypeID() { return m_ConnectInletTypeIDptr.ID; }
	CString GetConnectInletType();

	const IDPTR &GetConnectInletSizeIDPtr();
	CString GetConnectInletSizeID() { return m_ConnectInletSizeIDptr.ID; }
	CString GetConnectInletSize();

	CString GetConnectInlet();

	const IDPTR &GetConnectOutletTypeIDPtr();
	CString GetConnectOutletTypeID() { return m_ConnectOutletTypeIDptr.ID; }
	CString GetConnectOutletType();

	const IDPTR &GetConnectOutletSizeIDPtr();
	CString GetConnectOutletSizeID() { return m_ConnectOutletSizeIDptr.ID; }
	CString GetConnectOutletSize();

	CString GetConnectOutlet();

	bool GetHasHandwheel() { return m_bHasHandwheel; }
	CString GetHasHandwheelString();

	void SetSupplyOrLockshield( int iSupplyOrLockshield ) { m_iSupplyOrLockshield = iSupplyOrLockshield; }
	void SetConnectInletTypeID( LPCTSTR ptcConnectInletTypeID );
	void SetConnectInletSizeID( LPCTSTR ptcConnectInletSizeID );
	void SetConnectOutletTypeID( LPCTSTR ptcConnectOutletTypeID );
	void SetConnectOutletSizeID( LPCTSTR ptcConnectOutletSizeID );
	void SetHasHandhweel( bool bHasHandwheel ) { m_bHasHandwheel = bHasHandwheel; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////
	
// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int m_iSupplyOrLockshield;
	IDPTR m_ConnectInletTypeIDptr;
	IDPTR m_ConnectInletSizeIDptr;
	IDPTR m_ConnectOutletTypeIDptr;
	IDPTR m_ConnectOutletSizeIDptr;
	bool m_bHasHandwheel;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_FloorHeatingController : Floor heating controller
//
//	new:CDB_FloorHeatingController(ID)->FLOORHCONTROL_TAB
//	{
//		{
//			{
//				{
//					0				isDeleted
//					1				isAvailable
//					0				isHidden
//					1				isFix
//					0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//					0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//					0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//				}
//				Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//				PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//				Tmax in �C
//				Tmin in �C
//			}
//			"Reference"				Reference to the TA Product
//			"Name"					Name of the product
//			TypeID					Identifier of the valve type 
//			FamilyID				Identifier of the family of the product
//			VersionID				Identifier of the version of the product
//			PriorityLevel			Level of priority for selection [1,10]
//			Comment					Product comment
//			ProdImgID				Product image ID
//			AccessGroupID			Identifier of the accessory group
//			PartOfaSet				Boolean indicating if the valve is part of a ActSet
//			Price					Price of the product
//		}
//	}
//
////////////////////////////////////////////////////////////////

class CDB_FloorHeatingController : public CDB_Product
{
	DEFINE_DATA( CDB_FloorHeatingController )

protected:
	CDB_FloorHeatingController( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_FloorHeatingController() {}

// Public methods.
public:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_TapWaterControl : Tap water control valves
//
//	new:CDB_TapWaterControl(ID)->TAPCONTROL_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//					"Reference"				Reference to the TA Product
//					"Name"					Name of the product
//					TypeID					Identifier of the valve type 
//					FamilyID				Identifier of the family of the product
//					VersionID				Identifier of the version of the product
//					PriorityLevel			Level of priority for selection [1,10]
//					Comment					Product comment
//					ProdImgID				Product image ID
//					AccessGroupID			Identifier of the accessory group
//					PartOfaSet				Boolean indicating if the valve is part of a ActSet
//					Price					Price of the product
//				}
//				BodyMaterialID			Identifier of the body material
//				SizeID					Identifier of the corresp. valve size
//				ConnectID				Identifier of the (inlet/pipe side) connection
//				ConnTabID				Identifier of the connection table, can be NO_ID
//				Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//				CBIType					Type of the valve as used in the CBI II
//				CBISize					Size of the valve as used in the CBI II
//				CBISizeInch				Size of the valve in inch as used in the CBI II
//				CBICompatibilityIndex	CBI compatibility Index
//				CBIVersion				CBI Version
//				Dpmax					Maximum Dp of the valve
//				MeasPt					Measuring points
//				MeasurableData			Measurable data
//				InletSizeID				Identifier of the inlet size ID
//				OutletSizeID			Identifier of the outlet size ID
//			}
//			CharactID				Identifier of the valve characteristic
//		}
//		MinTemp					Min. temperature of the adjustable range.
//		MaxTemp					Max. temperature of the adjustable range.
//      HotWaterMainsSizeID		Identifier of the hot water mains input size ID.
//	}
//
////////////////////////////////////////////////////////////////

class CDB_TapWaterControl : public CDB_RegulatingValve
{
	DEFINE_DATA( CDB_TapWaterControl )

protected:
	CDB_TapWaterControl( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_TapWaterControl() {}

// Public methods.
public:
	double GetMinTempOfAdjustableRange() { return m_dMinTempOfAdjustableRange; }
	double GetMaxTempOfAdjustableRange() { return m_dMaxTempOfAdjustableRange; }
	IDPTR  GetHotWaterMainsSizeIDPtr();

	void SetMinTempOfAdjustableRange( double dMinTempOfAdjustableRange ) { if( dMinTempOfAdjustableRange != m_dMinTempOfAdjustableRange ) { m_dMinTempOfAdjustableRange = dMinTempOfAdjustableRange; Modified(); } }
	void SetMaxTempOfAdjustableRange( double dMaxTempOfAdjustableRange ) { if( dMaxTempOfAdjustableRange != m_dMaxTempOfAdjustableRange ) { m_dMaxTempOfAdjustableRange = dMaxTempOfAdjustableRange; Modified(); } }
	void SetHotWaterMainsSizeID( _string strHotWaterMainsSizeID ) { m_HotWaterMainsSizeIDPtr.SetID( strHotWaterMainsSizeID ); }

	CString GetTempAdjustableRangeStr( bool bWithUnits = false );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	double m_dMinTempOfAdjustableRange;
	double m_dMaxTempOfAdjustableRange;
	IDPTR  m_HotWaterMainsSizeIDPtr;
};
#endif	// TACBX


////////////////////////////////////////////////////////////////
//
//		CDB_DpCBCValve : Dp-Controller and Balancing control valve.
//      Ex: TA-COMPACT-DP
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_DpCBCValve(ID)->DPCBCVALV_TAB
//	{
//		{
//			{
//				{
//					{
//						{
//							{
//								0				isDeleted
//								1				isAvailable
//								0				isHidden
//								1				isFix
//								0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation )
//								0				Free integer (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//								0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//							}
//							Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//							PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//							Tmax in �C
//							Tmin in �C
//						}
//						"Reference"				Reference to the TA Product
//						"Name"					Name of the product
//						TypeID					Identifier of the valve type 
//						FamilyID				Identifier of the family of the product
//						VersionID				Identifier of the version of the product
//						PriorityLevel			Level of priority for selection [1,10]
//						Comment					Product comment
//						ProdImgID				Product image ID
//						AccessGroupID			Identifier of the accessory group
//						PartOfaSet				Boolean indicating if the valve is part of a ActSet
//						Price					Price of the product
//					}
//					BodyMaterialID			Identifier of the body material
//					SizeID					Identifier of the corresp. valve size
//					ConnectID				Identifier of the (inlet/pipe side) connection
//					ConnTabID				Identifier of the connection table, can be NO_ID
//					Conn2ID					Identifier of the outlet/body side connection, can be NO_ID
//					CBIType					Type of the valve as used in the CBI II
//					CBISize					Size of the valve as used in the CBI II
//					CBISizeInch				Size of the valve in inch as used in the CBI II
//					CBICompatibilityIndex	CBI compatibility Index
//					CBIVersion				CBI Version
//					Dpmax					Maximum Dp of the valve
//					MeasPt					Measuring points
//					MeasurableData			Measurable data
//					InletSizeID				Identifier of the inlet size ID
//					OutletSizeID			Identifier of the outlet size ID
//				}
//				CharactID				// MUST BE SET TO "NO_ID" -> valve characteristic is set now in PICVCharID
//			}
//			Kvs						Kvs of the CV
//			Stroke					Stroke of the CV
//			Rangeability			Rangeability of the CV, introduce as a double or as a string ">100" only '>' is supported
//			LeakageRate				Leakage rate of the CV as a fraction of Kvs
//			CloseOffCharID			Identifier of the close-off characteristic
//			ActuatorGroupID			Identifier of the actuator group
//			CvAdapterGroupID		Identifier of the CV adapter group
// 			CtrlPropID				Identifier of the control properties
//			AdapterListMode			Set what is the method to use to retrieve adapter list linked to this control valve (#eCVAdapterListNormal or #eCVAdapterListSpecial).
//									(see enum in 'tadb.txt' for more description).
//		}
//		DPCBCVCharID		Identifier of the DPCBCV characteristic
//	}
//
////////////////////////////////////////////////////////////////
class CDB_DpCBCValve : public CDB_ControlValve
{
	DEFINE_DATA( CDB_DpCBCValve )

protected:
	CDB_DpCBCValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_DpCBCValve() {}
	
// Public methods.
public:
	// Retrieve the fully qualified pointer to PICV characteristic.
	CDB_DpCBCVCharacteristic *GetDpCBCVCharacteristic();

	// Retrieve the fully qualified pointer to the valve characteristic define in the 'CDB_DpCBCVCharacteristic'.
	// Remark: overriding 'CDB_TAProduct' base class.
	// Remark: for 'CDB_DpCBCValve', 'CDB_RegulatingValve::CharactID' is set to 'NO_ID' because 'CDB_DpCBCVCharacteristic' is inherited from
	//         'CDB_ValveCharacteristic'.
	virtual CDB_ValveCharacteristic *GetValveCharacteristic() { return GetDpCBCVCharacteristic(); }

	// Get minimum differential pressure over the DP-controller of the valve (control part excluded).
	// Calculated based on the flow and the Kvm found in the characteristic of the valve.
	// Returns -1.0 if error.
	double GetDppmin( double dFlow, double dRho );

	// Get the differential pressure that the DP-controller stabilize (That is in fact equals to the differential pressure over
	// the control part of the valve and the load).
	// The Dpk is interpolated as a function of the flow with the internal characteristic of the valve.
	double GetDpk( double dFlow );

	// Get the minimum needed available differential pressure for the circuit.
	// DPC-BCV-Hmin = Dpk + Dppmin 
	double GetHMin( double dFlow, double dRho );

	// Get the minimum differential pressure that the valve can stabilize on the load (excluded the control part of the valve).
	double GetDplmin( void );

	// Get the maximum differential pressure that the valve can stabilize on the load (excluded the control part of the valve).
	// Remark: for this kind of valve, the Dpl max is depending of the flow.
	double GetDplmax( double dFlow, double dRho );

	// Get the differential pressure over the control part of the valve.
	// Dpc = Dpk - Dpl
	double GetDpc( double dFlow, double dDpl );

	// Get the minimum differential pressure over the all valve (Dp and control part).
	// Dp = Dppmin + Dpc
	// Remark: when we have Dpl, we can determine Dpc that is in fact Dpk - Dpl.
	double GetDpmin( double dFlow, double dRho, double dDpl );

	// Get the setting.
	double GetSetting( double dFlow, double dRho, double dDpl );

	// Returns the max setting or -1.0 if error or no DpCBCV characteristic for this valve.
	double GetMaxSetting();

	// Returns Qmax or -1.0 if error or no DpCBCV characteristic for this valve.
	double GetQmax();

	// Returns a formated string with Dpl range or _T("") if error or no DpCBCV characteristic curve is defined for this valve.
	_string GetFormatedDplRange( double dFlow, double dRho, bool fWithUnit = true );

	// Return Valve Char data* if exist; used mainly to accelerate comparaison between char in TAP filling list.
	virtual CData *GetValveCharDataPointer( void ) { return GetDpCBCVCharacteristic(); }
	
	virtual bool IsaCV() { return false; }
	virtual bool IsaBCV() { return false; }
	virtual bool IsaDPCBCV() { return true; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	void SetDpCBCVCharacteristicID( LPCTSTR strDpCBCVCharacteristicID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX
	// Retrieve a pointer on table containing actuator set.
	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	TCHAR m_DPCBCVCharacteristicID[_ID_LENGTH + 1];				// The DpCBCV characteristic ID.
	CDB_DpCBCVCharacteristic *m_pDpCBCVCharacteristic;			// Pointer on the DpCBCV characteristic.
};


////////////////////////////////////////////////////////////////
//
//		CDB_RuledTable :
//
////////////////////////////////////////////////////////////////
//	Inherit from CTable
//
//			ACXXXID \ &ACYYYID	// ACXXXID implies ACYYYID
//			ACXXXID \ !ACYYYID	// ACXXXID excludes ACYYYID
//			>ACXXXID			// ACXXXID is built-in
//			^ACXXXID			// ACXXXID excludes all others
//		  	*ACXXXID			// ACXXXID two pieces should be included
//			#ACXXXID			// ACXXXID is distributed on each parent item (3 vessels (1 primary + 2 secondary) => 3 times the accessory).
//			+ACXXXID 			// ACXXXID is an intersection between this group and an other. For this group accessory is NOT displayed
//			++ACXXXID			// ACXXXID is an intersection between this group and an other. For this group accessory is displayed
//
// Detail on '+' and '++'. It's typically used for example with the stem heater accessory. This accessory is to placed between a valve and the actuator.
// When we show these products, we will not show the steam accessory two times (One with the valve and one with the actuator).
// Valve will have an accessory group with for example "++ACSTEMHEATER_65_125" in it. It means that we can show the stem heater as accessory for this valve.
// Actuator will have an acessory group with for example "+ACSTEMHEATER_65_125" in it. Because only one '+' we specify that we don't want to show this accessory
// in the individual selection (But well in direct selection for actuator selection alone).
// If user choose a valve and an actuator which has not this accessory, the stem accessory is disabled. Otherwise, the stem accessory is available.
//
////////////////////////////////////////////////////////////////
#define CDB_RTCHAR_IMPLICATED	'&'
#define CDB_RTCHAR_EXCLUDED		'!'
#define CDB_RTCHAR_BUILTIN		'>'
#define CDB_RTCHAR_EXCLUDEALL	'^'
#define CDB_RTCHAR_BYPAIR		'*'
#define CDB_RTCHAR_DISTRIBUTED	'#'
#define CDB_RTCHAR_INTERSECTION	'+' // HYS-1108: Manage crossing tables ( one '+': accessory is not displayed except in DirSel)

class CDB_RuledTableBase : public CTable
{
	DEFINE_DATA( CDB_RuledTableBase )

public:
	CDB_RuledTableBase( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_RuledTableBase() {}

// Public some definitions.
public:
	enum MMapName
	{
		Excluded	= 0,
		Implicated	= 1,
		BuiltIn		= 2,
		Crossing	= 3,
		CrossingDisp= 4,
		Last		= 5
	};
	enum eRuledTableType
	{
		erttDefault,
		erttExcluding,
		erttLast
	};

	struct RuledTableItem
	{
		IDPTR IDPtr;
		bool fByPair;
		bool fDistributed;
	};
	typedef std::pair<_string, RuledTableItem>		MapPair;
	typedef std::multimap<_string, RuledTableItem>	MMap;
	typedef MMap::iterator							MMIter;

// Public methods.
public:
	bool AddItem( _string strKeyID, IDPTR ObjectIDPtr, MMapName eMapName );
	bool AddItem( _string strKeyID, RuledTableItem rRuledTableItem, MMapName eMapName );
	
	int GetExcluded( CData *pKey, std::vector<CData *> *pSet );
	int GetImplicate( CData *pKey, std::vector<CData *> *pSet );
	int GetBuiltIn( std::vector<CData *> *pSet );

	bool IsExcluded( CData *pKey );
	bool IsImplicate( CData *pKey );
	bool IsBuiltIn( CData *pKey );
	// HYS-1108
	bool IsACrossingAcc( CData* pKey );
	bool IsACrossingAccDisplayed( CData* pKey );

	bool IsRuleExist( MapPair rule, MMapName eMapName, bool fCompareOnlyIDPtr = false );

	int GetVector( MMapName eMapName, CData *pKey, std::vector<CData *> *pSet );
	eRuledTableType GetTableType() { return m_eTableType; }
	bool IsByPair( _string ID );
	bool IsDistributed( _string ID );

	// Allow to merge 2 ruled table without duplicate values.
	// Remark: We use this kind of feature for the PM individual selection.
	//		   See for example the 'CRViewSSelPM::_PrepareTBPlenoAccessories' method.
	bool MergeWith( CDB_RuledTableBase *pclRuledTable );
	MMap *GetMap( MMapName eMapName ) { return &m_armapList[eMapName]; }

	// SETTER.
	void SetTableType( eRuledTableType RuledTableType ) { m_eTableType = RuledTableType; }

	int CopyMMap( MMapName eMapName, MMap *pDestinationMap );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

#ifndef TACBX
	// TO TEST.
	void Export( CString strFileName );
#endif

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual void OnDatabaseDestroy();
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	int _GetMMap( MMap *pMap, CData *pKey, std::vector<CData *> *pSet );
	void _Clear( void );

// Protected variables.
protected:
	MMap m_armapList[Last];
	std::map<_string, bool> m_mapAccessoryByPairFlag;
	std::map<_string, bool> m_mapAccessoryByDistributedFlag;
	eRuledTableType m_eTableType;
};


////////////////////////////////////////////////////////////////
//
// new:CDB_RuledTable(ID) -> TABLEID_WHERETOINSERT
// {
//		{
//			"Table description"
//			TrueParent				0/1: 1 if this table is the true parent of the IDs in it.
//			[ID1],
//			[ID2],
//			...
//			[IDn]
//		}
//		[Operator]IDName1 [ \ [Operator]IDName2 \ [Operator]IDName3 ...
//		[Operator]IDNameA [ \ [Operator]IDNameB \ [Operator]IDNameC ...
//		[Operator]IDNameA1 [ \ [Operator]IDNameA2 \ [Operator]IDNameA3 ...
// }
//
// Each ID can 'Implicated' or 'Excluded' other ID. Here are the list of all available operators:
//	& -> Implicated
//  ! -> Excluded
//  > -> Builtin
//  ^ -> Item excluded all other items
//  * -> Item is sold by two!
//	# -> Accessory distributed on each parent item (3 vessels (1 primary + 2 secondary) => 3 times the accessory) 
////////////////////////////////////////////////////////////////
class CDB_RuledTable : public CDB_RuledTableBase
{
	DEFINE_DATA( CDB_RuledTable )

public:
	CDB_RuledTable( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_RuledTable() {}

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

#ifndef TACBX
	virtual void InterpretInheritedData( CString *pStr );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

private:

#ifndef TACBX
	void _ReadIDText( CString strID );
#endif
};


#ifndef TACBX 
////////////////////////////////////////////////////////////////
//
//		CDB_Actuator
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_Actuator(ID)->ACTUATOR_TAB
//	{
//		--> CDB_Thing
//		--> CDB_HydroThing
//		--> CDB_Product
//		OpeningType			Opening type (Linear or Circular)
//		MaxForceTorque		Max force or torque developed by the actuator
//		MinLimitedStroke	Min. limited stroke of the actuator
//      LimitedStrokeMode	Limititation nof the stroke is electronic, mechanical, ...
//		MaxStroke			Max stroke of the actuator (in m for #eOPENTYPE_Linear; in � for #eOPENTYPE_Circ)
//		ActAdapterGroupID	Identifier of the actuator adapter group
//		MinEnvTemp			Min temperature for operating environment in �C
//		MaxEnvTemp			Max temperature for operating environment in �C
//	}
//
////////////////////////////////////////////////////////////////
class CDB_Actuator : public CDB_Product
{
	DEFINE_DATA(CDB_Actuator)

public:
	enum LimitedStrokeMode
	{
		LSM_No,
		LSM_Electronic,
		LSM_Mechanic,
	};

protected:
	CDB_Actuator( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Actuator() {}

// Public methods.
public:
	virtual double GetOrderKey( void );

	CDB_CloseOffChar::eOpenType GetOpeningType() { return m_eOpeningType; }
	
	int GetMaxForceTorqueNumber() { return m_ardMaxForceTorque.GetCount(); }
	double GetMaxForceTorque( int iIndex = 0 );
	CString GetMaxForceTorqueStr( bool bWithUnit, int iIndex = 0 );
	
	// HYS-1355: Add minimum limited stroke to be able to choose TA-Compact-P/TA-Modulator with TA-6-way valves in low flow.
	double	GetMinLimitedStroke() { return m_dMinLimitedStroke; }

	// HYS-1389: We add now this variable to know if the limitation of the stroke is purely mechanical or electronical.
	LimitedStrokeMode GetLimitedStrokeMode() { return m_eLimitedStrokeMode; }

	double	GetMaxStroke() { return m_dMaxStroke; }
	IDPTR GetActAdapterGroupIDPtr();
	double GetActTmaxAmbiant() { return m_TmaxAmbiant; }
	double GetActTminAmbiant() { return m_TminAmbiant; }

	// SETTER.
	void SetOpeningType( CDB_CloseOffChar::eOpenType eOpeningType ) { m_eOpeningType = eOpeningType; }
	void SetMaxForceTorque( CArray<double> &ardMaxForceTorque );
	void SetMinLimitedStroke( double dMinLimitedStroke ) { m_dMinLimitedStroke = dMinLimitedStroke; }
	void SetLimitedStrokeMode( LimitedStrokeMode eLimitedStrokeMode ) { m_eLimitedStrokeMode = eLimitedStrokeMode; }
	void SetMaxStroke( double dMaxStroke ) { m_dMaxStroke = dMaxStroke; }
	void SetActAdapterGroupID( LPCTSTR tcActAdapterGroupID ) { m_ActAdapterGroupIDPtr.SetID( tcActAdapterGroupID ); }
	void SetTmaxAmbiant( double dTmaxAmbiant ) { m_TmaxAmbiant = dTmaxAmbiant; }
	void SetTminAmbiant( double dTminAmbiant ) { m_TminAmbiant = dTminAmbiant; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	CDB_CloseOffChar::eOpenType m_eOpeningType;
	CArray<double> m_ardMaxForceTorque;
	double m_dMinLimitedStroke;
	LimitedStrokeMode m_eLimitedStrokeMode;
	double m_dMaxStroke;
	IDPTR m_ActAdapterGroupIDPtr;
	double m_TmaxAmbiant;
	double m_TminAmbiant;
};
#endif	// TACBX


#ifndef TACBX 
////////////////////////////////////////////////////////////////
//
//		CDB_ElectroActuator
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_ElectroActuator(ID)->ACTUATOR_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation)
//						0				Integer to exclude product for selection: 1 (Individual/Batch), 2 (Direct) or 4 (HMCalc). (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//      				0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the product.
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				TableSetID				Table ID where are sets if exist for the current product.
//				Price					Price of the product
//			}
//			OpeningType			Opening type (Linear or Circular)
//			MaxForceTorque		Max force or torque developed by the actuator
//		    MinLimitedStroke	Min. limited stroke of the actuator
//			MaxStroke			Max stroke of the actuator (in m for #eOPENTYPE_Linear; in � for #eOPENTYPE_Circ)
//			ActAdapterGroupID	Identifier of the actuator adapter group
//			MinEnvTemp			Min temperature for operating environment in �C
//			MaxEnvTemp			Max temperature for operating environment in �C
//		}
//		ActuatingTimes		List of actuating times separated by "\" with default setting followed by * (in s/m for #eOPENTYPE_Linear; in � for #eOPENTYPE_Circ)
//		IPxxAuto			Enclosure protection of the actuator in auto mode
//		IPxxManual			Enclosure protection of the actuator when using manual override
//		Power supply		List of accepted Power supplies separated by "\"
//		Frequency			Accepted frequency
//		Input signals		List of accepted input control signals separated by "\"
//		Output signals		List of accepted output control signals separated by "\"
//      Fail safe			1 if fail safe function, 0 if no fail safe function
//		OnOffPossible		1 if a 3 points or proportional actuator can work in On/Off mode otherwise 0.
//		RelayID				Type of relay
//      Binary input		Number of binary input.
//      Default return position #eDRP_None, #eDRP_Extending, #eDRP_Retracting or #eDRP_Configurable.
//	}
//
////////////////////////////////////////////////////////////////
class CDB_ElectroActuator : public CDB_Actuator
{
	DEFINE_DATA( CDB_ElectroActuator )

protected:
	CDB_ElectroActuator( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ElectroActuator() {}

// Public enumeration defines.
public:
	// HYS-1079: Fail-safe function is redefined and now we use default return position 
	enum DefaultReturnPosition
	{
		Undefined = -1,
		None = 0,
		Extending = 1,
		Retracting = 2,
		Configurable = 3,
		LastDRP
	};

	enum class FailSafeType
	{
		eFSTypeUndefined = -1,
		eFSTypeNone = 0,
		eFSTypeElectronic = 1,
		eFSTypeSpringReturn = 2,
		eFSTypeLast
	};
	
// Public methods.
public:
	virtual double GetOrderKey( void );

	// Allow to check if current actuator fits with all parameters.
	// Param: 'strPowerSupplyID' and 'strInputSignalID' are the power supply and input signal IDs to check with actuator.
	// Param: 'iFailSafeFct' the fail-safe value to check ( 1 when fail-safe true, 0 when fail-safe false, -1 when undefined) .
	// Param: 'eDRPFunctionToCheck' is the default return position function to test.
	// Param: 'eControlValveDRPFct' is the default return position function of the control valve on which we want to test 'eDRPFunctionToCheck'.
	// Param: 'CvCtrlType' is the type of the control that user wants to apply on the control valve.
	// Param: 'fDowngradeActuatorFunctionality' set to 'true' if actuator with the '3 points' input signal type can work with the 'On/Off control type,
	//        and if actuator with the 'Proportional' input signal type can work with the 'On/Off' and the '3 points' control type.
	// Param: 'RelayID' is the type of the relay that we want.
	bool IsActuatorFit( CString strPowerSupplyID, CString strInputSignalID, int iFailSafeFct, CDB_ControlValve::DRPFunction eDRPFunctionToCheck, 
			CDB_ControlValve::DRPFunction eControlValveDRPFct, CDB_ControlProperties::CvCtrlType CvCtrlType, bool bDowngradeActuatorFunctionality = true,
			CString strRelayID = _T(""), int iBinaryInputNumber = -1 );

	// Allow to check if actuator can work with the control type.
	// Param: 'fDowngradeActuatorFunctionality' set to 'true' if actuator with the '3 points' input signal type can work with the 'On/Off control type,
	//        and if actuator with the 'Proportional' input signal type can work with the 'On/Off' and the '3 points' control type.
	bool IsActuatorControlFit( CDB_ControlProperties::CvCtrlType CvCtrlType, bool bDowngradeActuatorFunctionality = true );

	double GetDefaultActuatingTime();
	int GetDefaultActuatingTimeIndex() { return m_ActTimeDefaultIndex; }
	double GetActuatingTime( int iPos );
	unsigned int GetNumOfActuatingTimes() { return (int)m_ArrayActTimes.GetCount(); }
	// Return a composed str with all available Actuating Time.
	// If dStroke = 0 return value from database s/mm or s/deg otherwise compute time needed for stroke.
	CString GetActuatingTimesStr( double dStroke = 0.0, bool bWithUnit = false, CString strSeparator = _T("; ") );
	int GetActTimeDefIndex() { return m_ActTimeDefaultIndex; }

	CString GetIPxxAuto() { return CString( m_IPxxAuto ); }
	CString GetIPxxManual() { return CString( m_IPxxManual ); }
	CString GetIPxxFull();

	bool IsPowerSupplyAvailable( IDPTR idptr );
	unsigned int GetNumOfPowerSupplyIDptr() { return (int)m_PowerSupplyIDPtr.GetCount(); }
	IDPTR GetPowerSupplyIDPtr( unsigned int index );
	CString GetPowerSupplyStr( CString strSeparator = _T("; ") );

	IDPTR GetFrequencyIDPtr();

	bool IsInputSignalAvailable( IDPTR idptr );
	unsigned int GetNumOfInputSignalsIDptr() { return (int)m_InputSignals.GetCount(); }
	IDPTR GetInputSignalsIDPtr( unsigned int index );
	CString GetInOutSignalsStr( bool bInput, CString strSeparator = _T("; ") );

	bool IsOutputSignalAvailable( IDPTR idptr );
	unsigned int GetNumOfOutputSignalsIDptr() { return (int)m_OutputSignals.GetCount(); }
	IDPTR GetOutputSignalsIDPtr( unsigned int index );

	static _string GetDefaultReturnPosStr( DefaultReturnPosition eDefaultPos );
	DefaultReturnPosition GetDefaultReturnPos( void ) { return m_eDefaultReturnPos; }
	FailSafeType GetFailSafe( void ) { return m_eFailSafe; }

	bool IsOnOffPossible( void ) { return m_bOnOffPossible; }

	IDPTR GetRelayIDPtr();
	CString GetRelayID();
	CString GetRelayStr();

	int GetBinaryInputNumber() { return m_iBinaryInputNumber; }

	// SETTER.
	void SetActTimeDefaultIndex( int iActTimeDefaultIndex ) { m_ActTimeDefaultIndex = iActTimeDefaultIndex; }
	void SetArrayActTimes( CArray<double> &ArrayActTimes );
	void SetIPxxAuto( LPCTSTR tcIPxxAuto ) { _tcsncpy_s( m_IPxxAuto, SIZEOFINTCHAR( m_IPxxAuto ), tcIPxxAuto, SIZEOFINTCHAR( m_IPxxAuto ) ); }
	void SetIPxxManual( LPCTSTR tcIPxxManual ) { _tcsncpy_s( m_IPxxManual, SIZEOFINTCHAR( m_IPxxManual ), tcIPxxManual, SIZEOFINTCHAR( m_IPxxManual ) ); }
	void SetPowerSupplyIDPtr( CArray <IDPTR> &arPowerSupplyIDPtr );
	void SetFrequencyID( LPCTSTR tcFrequencyID ) { m_FrequencyIDPtr.SetID( tcFrequencyID ); }
	void SetInputSignals( CArray <IDPTR> &arInputSignals );
	void SetOutputSignals( CArray <IDPTR> &arOutputSignals );
	void SetFailSafeType( FailSafeType eFailSafe ) { m_eFailSafe = eFailSafe; } 
	void SetOnOffPossible( bool bOnOffPossible ) { m_bOnOffPossible = bOnOffPossible; }
	void SetRelayID( LPCTSTR tcRelayID ) { m_RelayIDPtr.SetID( tcRelayID ); }
	void SetBinaryInputNumber( int iBinaryInputNumber ) { m_iBinaryInputNumber = iBinaryInputNumber; }
	void SetDefaultReturnPosition( DefaultReturnPosition eDefaultReturnPos ) { m_eDefaultReturnPos = eDefaultReturnPos; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );

	// HYS-1456 : We added Power supply column in the filter tab for electro actuator
	virtual CheckFilterReturnValue CheckFilter( CFilterTab* pclFilter, CString strFilter );
	///////////////////////////////////////////////////////////////////////////////////

// Private members.
private:
	// Read double list, number marked with a * is considered as default value.
	void _ParseActuatingTimesList( CArray<double>*pAr, int &pos, CString list );

// Protected variables.
protected:
	int m_ActTimeDefaultIndex;
	CArray <double> m_ArrayActTimes;
	TCHAR m_IPxxAuto[8];
	TCHAR m_IPxxManual[8];
	CArray <IDPTR> m_PowerSupplyIDPtr;
	IDPTR m_FrequencyIDPtr;
	CArray <IDPTR> m_InputSignals;
	CArray <IDPTR> m_OutputSignals;
	FailSafeType m_eFailSafe;
	bool m_bOnOffPossible;
	IDPTR m_RelayIDPtr;
	int m_iBinaryInputNumber;
	DefaultReturnPosition m_eDefaultReturnPos;
};
#endif	// TACBX


#ifndef TACBX 
////////////////////////////////////////////////////////////////
//
//		CDB_ThermostaticActuator
//
////////////////////////////////////////////////////////////////
//
//	new:CDB_ThermostaticActuator(ID)->ACTUATOR_TAB
//	{
//		{
//			{
//				{
//					{
//						0				isDeleted
//						1				isAvailable
//						0				isHidden
//						1				isFix
//						0				Integer for flagging b31...b0 (bit 0: InCbi, bit1: available for Hub, bit2: available for HubStation)
//						0				Integer to exclude product for selection: 1 (Individual/Batch), 2 (Direct) or 4 (HMCalc). (bit 0: NotForIndSel, bit 1: NotForDirSel, bit 2: NotForHMCalc)
//      				0               Sort integer (Allow to show a group of articles in the order defined with this variable).
//					}
//					Pmaxmin and Pmaxmax in Pa (if Pmaxmin = PN6, Pmaxmin is not used in PN string; a 'd' in front of Pmaxmin enforces it in PN string)
//					PNRangeID			Identifier of CDB_RuledTable that contains available PN, could be NO_ID in this case Pmaxmin/max are used
//					Tmax in �C
//					Tmin in �C
//				}
//				"Reference"				Reference to the product.
//				"Name"					Name of the product
//				TypeID					Identifier of the valve type 
//				FamilyID				Identifier of the family of the product
//				VersionID				Identifier of the version of the product
//				PriorityLevel			Level of priority for selection [1,10]
//				Comment					Product comment
//				ProdImgID				Product image ID
//				AccessGroupID			Identifier of the accessory group
//				PartOfaSet				Boolean indicating if the valve is part of a ActSet
//				TableSetID				Table ID where are sets if exist for the current product.
//				Price					Price of the product
//			}
//			OpeningType			Opening type (Linear or Circular)
//			MaxForceTorque		Max force or torque developed by the actuator
//		    MinLimitedStroke	Min. limited stroke of the actuator
//			MaxStroke			Max stroke of the actuator (in m for #eOPENTYPE_Linear; in � for #eOPENTYPE_Circ)
//			ActAdapterGroupID	Identifier of the actuator adapter group
//			MinEnvTemp			Min temperature for operating environment in �C
//			MaxEnvTemp			Max temperature for operating environment in �C
//		}
//		MinSetting			Min setting temperature (�C)
//		MaxSetting			Max setting temperature (�C)
//		CapillaryLength		Length of the capillary tube if exist
//		Hysteresis			Hysteresis in K
//		FrostProtection		Frost protection temperature (�C)
//	}
//
////////////////////////////////////////////////////////////////
class CDB_ThermostaticActuator : public CDB_Actuator
{
	DEFINE_DATA( CDB_ThermostaticActuator )

protected:
	CDB_ThermostaticActuator( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_ThermostaticActuator() {}

// Public methods.
public:
	virtual double GetOrderKey( void );

	int GetMinSetting( void ) { return m_iMinSetting; }
	int GetMaxSetting( void ) { return m_iMaxSetting; }
	int GetCapillaryLength( void ) { return m_iCapillaryLength; }
	double GetHysteresis( void ) { return m_dHysteresis; }
	int GetFrostProtection( void ) { return m_iFrostProtection; }

	// SETTER
	void SetMinSetting( int iMinSetting ) { m_iMinSetting = iMinSetting; } 
	void SetMaxSetting( int iMaxSetting ) { m_iMaxSetting = iMaxSetting; } 
	void SetCapillaryLength( int iCapillaryLength ) { m_iCapillaryLength = iCapillaryLength; } 
	void SetHysteresis( double dHysteresis ) { m_dHysteresis = dHysteresis; } 
	void SetFrostProtection( int iFrostProtection ) { m_iFrostProtection = iFrostProtection; } 

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void InterpretInheritedData( CString *pStr );
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM  inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int m_iMinSetting;
	int m_iMaxSetting;
	int m_iCapillaryLength;
	double m_dHysteresis;
	int m_iFrostProtection;
};
#endif	// TACBX


#ifndef TACBX
class CDB_Set;
class CTableSet : public CTable
{
	DEFINE_DATA( CTableSet )

// Constructor and destructor.
protected:
	CTableSet( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CTableSet() {}

// Public methods.
public:
	// Return an array of CDB_Set *
	// FirstID or SecondID could be "" in this case they are not tested
	int FindCompatibleSet( std::set<CDB_Set *> *pSet, CString strFirstID, CString strSecondID, int eFilterSelection = 0 );
	
	// HYS-1877: This function allows to find PIBCV Set linked with 6-way valve tab.
	CDB_Set* FindCompatibleSet6WayValve( CString strFirstID, CString strSecondID );

	// Return first CDB_Set * corresponding to the IDs
	// FirstID or SecondID could be "" in this case they are not tested
	CDB_Set *FindCompatibleSet( CString strFirstID, CString strSecondID );

	// HYS-1388
	// Return first CDB_Set * corresponding to the IDs and accept set that are no more available.
	// In this case, unavailable set can be managed otherwise
	// FirstID or SecondID could be "" in this case they are not tested
	CDB_Set *FindSet( CString strFirstID, CString strSecondID );
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////
//
//		CDB_Set :
//
//	new:CDB_Set(ID)
//	{
//		--> CDB_Thing
//		"Reference"			Reference to the TA Product
//		"Name"				Name of the product
//		CVID				Identifier of the control valve
//		ACTID				Identifier of the actuator
//		AccessGroupID		Identifier of the accessory group
//		"Parameter"			String to allow to apply some filter.
//		"NextID"			Identifier of the next ID to consider can be Set ID, Group ID, Product ID etc...(HYS-1877)
//	}
//
////////////////////////////////////////////////////////////////
class CDB_Set : public CDB_Thing
{
	DEFINE_DATA( CDB_Set )

protected:
	CDB_Set( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Set() {}

// Public methods.
public:
	CString GetReference();
	// HYS-1877: To get localized name.
	CString GetName();
	IDPTR GetFirstIDPtr() { return GetMbrIDPtr( m_FirstDPtr ); }
	IDPTR GetSecondIDPtr() { return GetMbrIDPtr( m_SecondIDPtr ); }
	IDPTR GetAccGroupIDPtr() { return GetMbrIDPtr( m_AccGroupIDPtr ); }
	CString GetParameter() { return m_strParameter; }
	CString GetNextID() { return m_strNextID; }

	// Overrides 'CData' public virtual methods.
	virtual LPCTSTR GetArtNum( bool fWithoutConnection = false ) { return GetReference(); }

	// SETTER.
	void SetReference( CString strReference ) { m_strReference = strReference; }
	void SetName( CString strName ) { m_strName = strName; }
	void SetFirstID( LPCTSTR tcFirstID ) { m_FirstDPtr.SetID( tcFirstID ); }
	void SetSecondID( LPCTSTR tcSecondID ) { m_SecondIDPtr.SetID( tcSecondID ); }
	void SetAccGroupID( LPCTSTR tcAccGroupID ) { m_AccGroupIDPtr.SetID( tcAccGroupID ); }
	void SetParameter( CString strParameter ) { m_strParameter = strParameter; }
	void SetNextID( CString strNextID ) { m_strNextID = strNextID; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	virtual void DeleteArticle( void );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	IDPTR GetMbrIDPtr( IDPTR &idptr );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void InterpretInheritedData(CString *pStr);
	virtual void WriteText( OUTSTREAM outf, CString &strTab );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	virtual CheckFilterReturnValue CheckFilter( CFilterTab *pclFilter, CString strFilter );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	CString m_strReference;
	CString m_strName;
	IDPTR m_FirstDPtr;
	IDPTR m_SecondIDPtr;
	IDPTR m_AccGroupIDPtr;
	CString m_strParameter;
	CString m_strNextID;
	_string *m_pStr;
};
#endif	// TACBX


#ifndef TACBX
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//								CDB_PageField
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// !!!! Keep historic version to ease conversion when reading.
// The history is saved in the "DBPageFieldVersion.h" file.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This enum allows us to hide/show some variables or group of product in the "Result page" in HySelect.
// In the left panel, there is the "Format columns..." under the "Displayed information" title.
// A dialog appears and contains exactly the structure that is defined with this enum (The structure is created in the
// 'CDlgPageField::OnInitDialog' method).
//
//		First reference
//		Second reference
//		Water information
//		+ Separator & Air vents information
//		+ Vessel information
//		...
//
// If you deploy for example "Safety valve information", you can remark that we can have one or more (Generally only 2)
// subgroups "Product" and "Technical". If you deploy boht you have:
//      - Safety valve information
//         - Product
//         |   |_ Name
//		   |   |_ Size
//		   |   |_ Connection
//		   |   |_ Application type
//         - Technical 
//             |_ Set pressure
//             |_ Heat generator type
//             |_ Max. power
//             |_ Power system
//             |_ Max. collector
//             |_ Collector 
//			
// If you take a look in the "PageField_enum" below for the safety valves we have:
//      epfSAFETYVALVEINFO,									-> Safety valve information
//			epfSAFETYVALVEINFOPRODUCT,							-> Product
//				epfSAFETYVALVEINFOPRODUCTNAME,						-> Name
//              epfSAFETYVALVEINFOPRODUCTSIZE,						-> Size
//				epfSAFETYVALVEINFOPRODUCTCONNECTION,				-> Connection
//				epfSAFETYVALVEINFOPRODUCTPROJECTTYPE,				-> Application Type
//			epfSAFETYVALVETECHINFO,								-> Technical
//				epfSAFETYVALVETECHINFOSETPRESSURE,					-> Set pressure
//				epfSAFETYVALVETECHINFOHEATGENERATORTYPE,			-> Heat generator type
//				epfSAFETYVALVETECHINFOPOWERMAX,						-> Max. power
//				epfSAFETYVALVETECHINFOPOWERSYSTEM,					-> Power system
//				epfSAFETYVALVETECHINFOCOLLECTORMAX,                 -> Max. collector
//				epfSAFETYVALVETECHINFOCOLLECTOR,					-> Collector

// Remark, if you need to add a new version, let's say 28:

// First step.
// Copy all the current 'PageField_enum' below in the 'DBPageFieldVersion.h" and add "_V27" at the end of each enum:
//		enum PageField_enum_V27 {
//			epfNONE_V27,
//			epfFIRSTREF_V27,
//			epfSECONDREF_V27,
//			epfWATERINFO_V27,
//			epfSEPARATORAIRVENTINFO_V27,
//				epfSEPARATORAIRVENTINFOPRODUCT_V27,
//		...
//		epfLASTFIELD_V27
//		};

// Second step.
// Now add your variables in the 'PageField_enum' below.
// It can be a new group or just a few additional information.
// For group, you have to respect the structure detailed below. The 'Product' subgroup must always exists
// 'Technical' is optional if there is no technical information
// Example for a group:
//		epfNEWPRODUCTINFO,
//			epfNEWPRODUCTINFOPRODUCT,
//			epfNEWPRODUCTINFOPRODUCTNAME,
//			epfNEWPRODUCTINFOPRODUCTSIZE,
//		epfNEWPRODUCTINFOTECHINFO,
//			epfNEWPRODUCTINFOINFOPS,
//
// Example for just a new variable in a existing group (for example "epfBLOWTANKINFOPRODUCTPN").
//
//		epfBLOWTANKINFO,									// V17: Blow tank product information
//		epfBLOWTANKINFOPRODUCT,								// V17
//			epfBLOWTANKINFOPRODUCTNAME,						// V17
//			epfBLOWTANKINFOPRODUCTSIZE,						// V17
//			epfBLOWTANKINFOPRODUCTPN,						// V28
//		epfBLOWTANKTECHINFO,								// V17
//			epfBLOWTANKTECHINFOPS,							// V17
//
// Case 1: We can insert a group at the very end of the "PageField_enum".
// Case 2. ATTTENTION! 
//   If you insert a group or a variable everywhere in the "PageField_enum" existing enum, take a note of
//   the name of the enum that is just before where you want to insert your group or variable.
// For example: a) imagine we insert all the "epfNEWPRODUCTINFO" group just before the safety valve information, it is just before the "epfSAFETYVALVEINFO", keep this name,
//                 it will be used in the "CDB_PageField::SetPredefStyle" and "CDB_PageField::Read" methods.
//              b) The same for a new variable in an existing group. Here we have inserted "epfBLOWTANKINFOPRODUCTPN" just before
//				   the "epfBLOWTANKTECHINFO" name. Keep this name in mind.

// Third step.
// Update the "EnumeratorCount" enum by adding a new line for the new version:
//		eECVersion28 = XXX,
// To determine the number to put for "XXX" you can point your mouse pointer on the "epfLASTFIELD" field of the "PageField_enum" enum and we will see a popup with the new value
// Or you can just count the number of new lines you have added and add to the previous "eECVersion27" version.
// Or to be very sure you can copy all the "PageField_enum" enum from "epfNONE" to "epfLASTFIELD" included and paste in a text monitor.
// Pay attention that generally line number for editor are 1-indexed. So the number of lines must be decremented of 1.
// If we have 231 lines, the "m_Field" array in the "PageField_struct" structure defined below will have a range of 0-230.

// Fourth step.
// Update info in the "CDB_PageField::SetPredefStyle" method -> see comment in the start of this method.

// Fifth step.
// Update the version of the "CDB_PageField::Write" method: "#define CDB_PAGEFIELD_VERSION	27" becomes "#define CDB_PAGEFIELD_VERSION	28"

// Sixth step.
// Update the "CDB_PageField::Read" method -> see comment in the end of this method.

// Version 26: we now show pressure drop for the smart control valves.
// Version 27: HYS-2067: we now show flow for smart Dp.
enum PageField_enum {
	epfNONE,
	epfFIRSTREF,											// ??: First Reference
	epfSECONDREF,											// ??: Second Reference
	epfWATERINFO,											// ??: Water information
	epfSEPARATORAIRVENTINFO,								// V8: Separator information -> V10 name changed (v8: epfSEPARATORINFO)
		epfSEPARATORAIRVENTINFOPRODUCT,						// V10 name changed (v8: epfSEPARATORINFOPRODUCT)
			epfSEPARATORAIRVENTINFOPRODUCTNAME,				// V10 name changed (v8: epfSEPARATORINFOPRODUCTNAME)
			epfSEPARATORAIRVENTINFOPRODUCTDPP,				// V11
			epfSEPARATORAIRVENTINFOPRODUCTSIZE,				// V10 name changed (v8: epfSEPARATORINFOPRODUCTSIZE)
			epfSEPARATORAIRVENTINFOPRODUCTCONNECTION,		// V10 name changed (v8: epfSEPARATORINFOPRODUCTCONNECTION)
			epfSEPARATORAIRVENTINFOPRODUCTVERSION,			// V10 name changed (v8: epfSEPARATORINFOPRODUCTVERSION)
			epfSEPARATORAIRVENTINFOPRODUCTPN,				// V10 name changed (v8: epfSEPARATORINFOPRODUCTPN)
		epfSEPARATORAIRVENTTECHINFO,						// V10 name changed (v8: epfSEPARATORTECHINFO)
			epfSEPARATORAIRVENTTECHINFOFLOW,				// V10 name changed (v8: epfSEPARATORTECHINFOFLOW)
			epfSEPARATORAIRVENTTECHINFODP,					// V10 name changed (v8: epfSEPARATORTECHINFODP)
	epfSTATICOINFO,											// V9: Statico product information
		epfSTATICOINFOPRODUCT,								// V9
			epfSTATICOINFOPRODUCTNAME,						// V9
			epfSTATICOINFOPRODUCTCONNECTION,				// V9
			epfSTATICOINFOPRODUCTNOMINALVOLUME,				// V9
			epfSTATICOINFOPRODUCTMAXPRESSURE,				// V9
			epfSTATICOINFOPRODUCTTEMPRANGE,					// V9
			epfSTATICOINFOPRODUCTWEIGHT,					// V12
			epfSTATICOINFOPRODUCTMAXWEIGHT,					// V12
	epfTECHBOXINFO,											// V9: Tecbox product information
		epfTECHBOXINFOPRODUCT,								// V9
			epfTECHBOXINFOPRODUCTNAME,						// V9
			epfTECHBOXINFOPRODUCTPS,						// V9
			epfTECHBOXINFOPRODUCTPOWER,						// V9
			epfTECHBOXINFOPRODUCTSUPPLYVOLTAGE,				// V9
	epfWATERMAKEUPINFO,										// V13
		epfWATERMAKEUPINFOPRODUCT,							// V13
			epfWATERMAKEUPINFOPRODUCTNAME,					// V13
			epfWATERMAKEUPINFOPRODUCTFUNCTIONS,				// V13
			epfWATERMAKEUPINFOPRODUCTCAPACITY,				// V13
			epfWATERMAKEUPINFOPRODUCTHEIGHT,				// V13
			epfWATERMAKEUPINFOPRODUCTWEIGHT,				// V13
	epfSAFETYVALVEINFO,										// V16: Safety valve product information
		epfSAFETYVALVEINFOPRODUCT,							// V16
			epfSAFETYVALVEINFOPRODUCTNAME,					// V16
			epfSAFETYVALVEINFOPRODUCTSIZE,					// V16
			epfSAFETYVALVEINFOPRODUCTCONNECTION,			// V16
			epfSAFETYVALVEINFOPRODUCTPROJECTTYPE,			// V16
		epfSAFETYVALVETECHINFO,								// V16
			epfSAFETYVALVETECHINFOSETPRESSURE,				// V16
			epfSAFETYVALVETECHINFOHEATGENERATORTYPE,		// V16 - Only for heating.
			epfSAFETYVALVETECHINFOPOWERMAX,					// V16
			epfSAFETYVALVETECHINFOPOWERSYSTEM,				// V16
			epfSAFETYVALVETECHINFOCOLLECTORMAX,				// V16 - Only for solar.
			epfSAFETYVALVETECHINFOCOLLECTOR,				// V16 - Only for solar.
	epfBLOWTANKINFO,										// V17: Blow tank product information
		epfBLOWTANKINFOPRODUCT,								// V17
			epfBLOWTANKINFOPRODUCTNAME,						// V17
			epfBLOWTANKINFOPRODUCTSIZE,						// V17
		epfBLOWTANKTECHINFO,								// V17
			epfBLOWTANKTECHINFOPS,							// V17
	epfBVINFO,												// ??: BV Information
		epfBVINFOPRODUCT,
			epfBVINFOPRODUCTNAME,
			epfBVINFOPRODUCTSIZE,
			epfBVINFOPRODUCTCONNECTION,
			epfBVINFOPRODUCTVERSION,
			epfBVINFOPRODUCTPN,
		epfBVTECHINFO,
			epfBVTECHINFOFLOW,
			epfBVTECHINFODP,
			epfBVTECHINFOSETTING,
	epfDPCINFO,												// ??: Dpc Information
		epfDPCINFOPRODUCT,
			epfDPCINFOPRODUCTNAME,
			epfDPCINFOPRODUCTSIZE,
			epfDPCINFOPRODUCTCONNECTION,
			epfDPCINFOPRODUCTVERSION,						// V6
			epfDPCINFOPRODUCTDPLRANGE,
			epfDPCINFOPRODUCTPN,
		epfDPCTECHINFO,
			epfDPCTECHINFOFLOW,
			epfDPCTECHINFO2NDDP,
			epfDPCTECHINFOHMIN,
			epfDPCTECHINFOCONNECTSCHEME,
			epfDPCTECHINFOAUTHORITY,
			epfDPCTECHINFOSETTING,
	epfDPCBCVINFO,											// V14
		epfDPCBCVINFOPRODUCT,								// V14
			epfDPCBCVINFOPRODUCTNAME,						// V14
			epfDPCBCVINFOPRODUCTSIZE,						// V14
			epfDPCBCVINFOPRODUCTCONNECTION,					// V14
			epfDPCBCVINFOPRODUCTVERSION,					// V14
			epfDPCBCVINFOPRODUCTDPLRANGE,					// V14
			epfDPCBCVINFOPRODUCTPN,							// V14
		epfDPCBCVTECHINFO,									// V14
			epfDPCBCVTECHINFOFLOW,							// V14
			epfDPCBCVTECHINFODPMIN,							// V14
			epfDPCBCVTECHINFOSETTING,						// V14
			epfDPCBCVTECHINFOHMIN,							// V14
	epfSVINFO,												// V15
		epfSVINFOPRODUCT,									// V15
			epfSVINFOPRODUCTNAME,							// V15
			epfSVINFOPRODUCTSIZE,							// V15
			epfSVINFOPRODUCTCONNECTION,						// V15
			epfSVINFOPRODUCTVERSION,						// V15
			epfSVINFOPRODUCTPN,								// V15
		epfSVTECHINFO,										// V15
			epfSVTECHINFOFLOW,								// V15
			epfSVTECHINFODP,								// V15
	epfRD7INFO,												// ??: Radiator Set Information
		epfRD7INFOPRODUCT,
			epfRD7INFOPRODUCTNAME,
			epfRD7INFOPRODUCTSIZE,
			epfRD7INFOPRODUCTCONNECTION,
			epfRD7INFOPRODUCTVERSION,
			epfRD7INFOPRODUCTPN,
		epfRD7TECHINFO,
			epfRD7TECHINFOFLOW,
			epfRD7TECHINFODP,
			epfRD7TECHINFOSETTING,
			epfRD7TECHINFODPTOT,
			epfRD7TECHINFOCONNECTSCHEME,
			epfRD7TECHINFODPMIN,							// V7: Special for thermostatic with automatic flow limitation.
			epfRD7TECHINFOFLOWRANGE,						// V7: Special for thermostatic with automatic flow limitation.
		epfRD7RADINFO,
			epfRD7RADINFOREQPOW,
			epfRD7RADINFOINSTPOW,
			epfRD7RADINFOTROOM,
			epfRD7RADINFOTSUPPLY,
			epfRD7RADINFODELTAT,
	epfDSINFO,												// ??: Direct Selection informations
		epfDSINFOPRODUCT,
			epfDSINFOPRODUCTNAME,
			epfDSINFOPRODUCTSIZE,
			epfDSINFOPRODUCTCONNECTION,
			epfDSINFOPRODUCTVERSION,
			epfDSINFOPRODUCTPN,
	epfPDINFO,												// ??: Partially defined product
		epfPDINFOPRODUCT,
			epfPDINFOPRODUCTNAME,
			epfPDINFOPRODUCTSIZE,
	epfARTICLE,
	epfPIPEINFO,											// ??: Pipe Information
		epfPIPENAME,
		epfPIPESIZE,
		epfPIPEWATERU,
		epfPIPEDP,
	epfQUANTITY,											// ??: Quantity
	epfSALEINFO,											// ??: Sale information
		epfSALEUNITPRICE,
		epfSALETOTALPRICE,
	epfREMARK,												// ??: Remark
	epfDPCTECHINFODPMIN,									// ??: Dpmin for DPC
	epfDSTECHINFO,											// ??: Technical information for Direct Selection
	epfPDTECHINFO,											// ??: Technical information for partially defined product
	epfNU1,													// V4
	epfNU2,													// V4
	epfCVINFO,												// V4: Cv and PICv Information
		epfCVINFOPRODUCT,									// V4
			epfCVINFOPRODUCTNAME,							// V4
			epfCVINFOPRODUCTKV,								// V4
			epfCVINFOPRODUCTBDYMATERIAL,					// V4
			epfCVINFOPRODUCTCONNECTION,						// V4
			epfCVINFOPRODUCTVERSION,						// V4
			epfCVINFOPRODUCTPN,								// V4
		epfCVTECHINFO,										// V4
			epfCVTECHINFOFLOW,								// V4
			epfCVTECHINFODP,								// V4
			epfCVTECHINFOSETTING,							// V5
	epfSMARTCONTROLVALVEINFO,								// V20: Smart control valve information.
		epfSMARTCONTROLVALVEINFOPRODUCT,					// V20
			epfSMARTCONTROLVALVEINFOPRODUCTNAME,			// V20
			epfSMARTCONTROLVALVEINFOPRODUCTBDYMATERIAL,		// V20
			epfSMARTCONTROLVALVEINFOPRODUCTCONNECTION,		// V20
			epfSMARTCONTROLVALVEINFOPRODUCTPN,				// V20
			epfSMARTCONTROLVALVEINFOPRODUCTKVS,				// V20
			epfSMARTCONTROLVALVEINFOPRODUCTQNOM,			// V20
			epfSMARTCONTROLVALVEINFOPRODUCTPOWERSUPPLY,		// V22
			epfSMARTCONTROLVALVEINFOPRODUCTINPUTSIGNAL,		// V22
			epfSMARTCONTROLVALVEINFOPRODUCTOUTPUTSIGNAL,	// V22
		epfSMARTCONTROLVALVETECHINFO,						// V20
			epfSMARTCONTROLVALVETECHINFOFLOW,				// V20
			epfSMARTCONTROLVALVETECHINFODP,					// V26
			epfSMARTCONTROLVALVETECHINFODPMIN,				// V20
			epfSMARTCONTROLVALVETECHINFOLOCALIZATION,		// V23
			epfSMARTCONTROLVALVETECHINFOCONTROLMODE,		// V23
			epfSMARTCONTROLVALVETECHINFOPOWERMAX,			// V23
	epfSMARTDPCINFO,										// V25: Smart differential pressure controller information.
		epfSMARTDPCINFOPRODUCT,								// V25
			epfSMARTDPCINFOPRODUCTNAME,						// V25
			epfSMARTDPCINFOPRODUCTBDYMATERIAL,				// V25
			epfSMARTDPCINFOPRODUCTCONNECTION,				// V25
			epfSMARTDPCINFOPRODUCTPN,						// V25
			epfSMARTDPCINFOPRODUCTKVS,						// V25
			epfSMARTDPCINFOPRODUCTQNOM,						// V25
			epfSMARTDPCINFOPRODUCTDPMAX,					// V25
		epfSMARTDPCTECHINFO,								// V25
			epfSMARTDPCTECHINFOFLOW,						// V27
			epfSMARTDPCTECHINFOPOWERMAX,					// V27
			epfSMARTDPCTECHINFODP,							// V25
			epfSMARTDPCTECHINFODPMIN,						// V25
			epfSMARTDPCTECHINFODPMAX,						// V25
			epfSMARTDPCTECHINFOLOCALIZATION,				// V25
	epf6WAYVALVEINFO,										// V18: 6-way valve information
		epf6WAYVALVEINFOPRODUCT,							// V18
			epf6WAYVALVEINFOPRODUCTNAME,					// V18
			epf6WAYVALVEINFOPRODUCTKVS,						// V18
			epf6WAYVALVEINFOPRODUCTBDYMATERIAL,				// V18
			epf6WAYVALVEINFOPRODUCTCONNECTION,				// V18
			epf6WAYVALVEINFOPRODUCTVERSION,					// V18
			epf6WAYVALVEINFOPRODUCTPN,						// V18
		epf6WAYVALVETECHINFO,								// V18
			epf6WAYVALVETECHINFOFLOW,						// V18
			epf6WAYVALVETECHINFODP,							// V18
	epfTAPWATERCONTROLINFO,									// V19: Tap water control information
		epfTAPWATERCONTROLINFOPRODUCT,						// V19
			epfTAPWATERCONTROLINFOPRODUCTNAME,				// V19
			epfTAPWATERCONTROLINFOPRODUCTSETTINGRANGE,		// V19
			epfTAPWATERCONTROLINFOPRODUCTBDYMATERIAL,		// V19
			epfTAPWATERCONTROLINFOPRODUCTCONNECTION,		// V19
			epfTAPWATERCONTROLINFOPRODUCTVERSION,			// V19
	epfFLOORHEATINGCONTROLINFO,								// V21: Floor heating control information
		epfFLOORHEATINGCONTROLINFOPRODUCT,					// V21
			epfFLOORHEATINGCONTROLINFOPRODUCTNAME,			// V21
			epfFLOORHEATINGCONTROLINFOPRODUCTADDINFO,		// V21
	epfPRESSUREREDUCVALVEINFO,								// V24: HYS-1741: Accessories and services: Pressure reducing valve.
		epfPRESSUREREDUCVALVEINFOPRODUCT,					// V24
			epfPRESSUREREDUCVALVEINFOPRODUCTNAME,			// V24
			epfPRESSUREREDUCVALVEINFOPRODUCTBDYMATERIAL,	// V24
			epfPRESSUREREDUCVALVEINFOPRODUCTCONNECTION,	    // V24
			epfPRESSUREREDUCVALVEINFOPRODUCTVERSION,	    // V24
			epfPRESSUREREDUCVALVEINFOPRODUCTPN,	            // V24
			epfPRESSUREREDUCVALVEINFOPRODUCTINLETPRESS,     // V24
			epfPRESSUREREDUCVALVEINFOPRODUCTOUTLETPRESS,    // V24
	epfLASTFIELD
};
#define PAGEFIELD_SIZE	epfLASTFIELD

enum EnumeratorCount
{
	eECVersion1 = 59,
	eECVersion2 = 70,
	eECVersion3 = 75,		// In version 3 we had really 73 enums defined but we wrote 75 bytes in files.
	eECVersion4 = 86,
	eECVersion5 = 87,
	eECVersion6 = 88,
	eECVersion7 = 90,
	eECVersion8 = 108,
	eECVersion9 = 125,
	eECVersion10 = 117,
	eECVersion11 = 118,
	eECVersion12 = 116,
	eECVersion13 = 123,
	eECVersion14 = 136,
	eECVersion15 = 146,
	eECVersion16 = 159,
	eECVersion17 = 165,
	eECVersion18 = 176,
	eECVersion19 = 183,
	eECVersion20 = 195,
	eECVersion21 = 199,
	eECVersion22 = 202,
	// 06/05/2021: Release v4.5.0.0 
	// 14/06/2021: Add 4 fields for version 23: epfSMARTCONTROLVALVETECHINFOLOCALIZATION, epfSMARTCONTROLVALVETECHINFOCONTROLMODE, epfSMARTCONTROLVALVETECHINFOFLOWMAX and epfSMARTCONTROLVALVETECHINFOPOWERMAX
	//             eECVersion23 = 206 (eECVersion22 = 202).
	// 17/06/2021: Remove one filed for version 23: epfSMARTCONTROLVALVETECHINFOFLOWMAX.
	//             I should have changed eECVersion23 to 205 !!!!
	// 17/06/2021: Remove one field from version 20: epfSMARTCONTROLVALVEINFOPRODUCTACTUATORNAME
	//             This has been well managed in the "DataBObj.cpp" file in the "bool CDB_PageField::Read" method:
	//                  if( Version < 23 )
	//                  {
	//						// Version 23: remove 'epfSMARTCONTROLVALVEINFOPRODUCTACTUATORNAME_V22'.
	//						memmove( (PVOID)&m_Field[epfSMARTCONTROLVALVEINFOPRODUCTACTUATORNAME_V22], (const PVOID)&m_Field[epfSMARTCONTROLVALVETECHINFO_V22], eECVersion22 - epfSMARTCONTROLVALVETECHINFO_V22 + 1 );
	//
	//						// Version 23: Smart control valve localization, control mode and power max added before 'epf6WAYVALVEINFO_V22'.
	//                      memmove( (PVOID)&m_Field[epf6WAYVALVEINFO_V23], (const PVOID)&m_Field[epf6WAYVALVEINFO_V22], eECVersion22 - epf6WAYVALVEINFO_V22 );
	//
	//						m_Field[epfSMARTCONTROLVALVETECHINFOLOCALIZATION_V23] = 1;
	//						m_Field[epfSMARTCONTROLVALVETECHINFOCONTROLMODE_V23] = 1;
	//						m_Field[epfSMARTCONTROLVALVETECHINFOPOWERMAX_V23] = 1;
	//                  }
	//             In reality in the version 23 we have well a total of 4 fields added, minus epfSMARTCONTROLVALVETECHINFOFLOWMAX and minus epfSMARTCONTROLVALVETECHINFOFLOWMAX/
	//             So, eECVersion23 is well 204 !
	// 16/08/2021: Release v4.5.1.0 -> ALL IS OK !
	// 08/03/2023: I don't know why (FF), I have changed eECVersion23 from 204 to 205. Simply because by looking the "DBPageFieldVersion.h" I just saw:
	//					epfSMARTCONTROLVALVETECHINFO,						// V20
	//						epfSMARTCONTROLVALVETECHINFOFLOW,				// V20
	//						epfSMARTCONTROLVALVETECHINFODP,					// V26
	//						epfSMARTCONTROLVALVETECHINFODPMIN,				// V20
	//						epfSMARTCONTROLVALVETECHINFOLOCALIZATION,		// V23
	//						epfSMARTCONTROLVALVETECHINFOCONTROLMODE,		// V23
	//						epfSMARTCONTROLVALVETECHINFOPOWERMAX,			// V23
	//             Thus for me it was 3 variables added and not ONLY 2. Thus I have incremented from the eECVersion22 = 202 to eECVersion23 = 205 without taking account that
	//             really in this version 23 one other variable has been removed (epfSMARTCONTROLVALVEINFOPRODUCTACTUATORNAME_V22)
	eECVersion23 = 204,
	eECVersion24 = 213,
	eECVersion25 = 227,
	eECVersion26 = 228,
	eECVersion27 = 230,
	eECVersionLast = eECVersion27,
	eECVersionBiggest = eECVersion27,		// Must be set to the biggest one in list above!!
};

struct PageField_struct
{
	bool m_Field[eECVersionBiggest];	
};

#define PAGESTYLECOMPLETE		1
#define PAGESTYLETECHNICAL		2
#define PAGESTYLETECHNICALSHORT	3
#define PAGESTYLEORDER			4

class CDB_PageField : public CDB_String, protected PageField_struct
{
	DEFINE_DATA( CDB_PageField )

protected:
	CDB_PageField( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_PageField() {}

// Public methods.
public:
	bool CompareFields( CDB_PageField *pf );
	void ClearAllFields () { memset( m_Field, 0, PAGEFIELD_SIZE ); }

	void SetField( PageField_enum index,  bool fState ) { if( m_Field[index] != fState){ m_Field[index] = fState; Modified(); } }
	void SetPredefStyle( int iStyle );
	void SetStyle( CDB_PageField *pf ) { if( memcmp( m_Field, pf->m_Field, sizeof( PageField_struct ) ) ) { memcpy( m_Field, pf->m_Field, sizeof( PageField_struct ) ); Modified(); } }

	bool GetField( PageField_enum index ) { return m_Field[index]; }
	CDB_PageField *GetStyle( CDB_PageField *pf ) { memcpy( pf->m_Field, m_Field, sizeof( PageField_struct ) ); return(pf); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

	// Allow test unit to get access to the protected methods.
	void PublicWrite( OUTSTREAM outf ) { return Write( outf ); }
	bool PublicRead( INPSTREAM inpf ) { return Read( inpf ); }
	
// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction.
	virtual void OnDatabaseDestroy();

	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_PageSetup
//
//
///////////////////////////////////////////////////////////////////////////////////
#define _HEADER_LENGTH		250
#define _KEY_NUMBER			3						// number of sorting keys
#define _PAGESETUP_ENUM		4						// number of different fields
#define _CHECKBOXNUMBERv5	8						// number of fields present in enum enCheck
#define _CHECKBOXNUMBERv4	9						// number of fields present in enum enCheck
#define _CHECKBOXNUMBERv2	6						// number of fields present in enum enCheck
#define _CHECKBOXNUMBERv1	5						// number of fields present in enum enCheck
													// in version 1 of CDB_PageSetup
class CDB_PageSetup;

enum PageSetup_enum {
	epsFIRSTHEADER,
	epsHEADER,
	epsFIRSTFOOTER,
	epsFOOTER
};

enum PageSetup_Alignment {
	epaTextLeft,
	epaTextCenter,
	epaTextRight
};

struct PageSetup_struct
{
	TCHAR				m_tzName[_STRING_LENGTH + 1];	// Page Setup Name
	PageField_enum		m_areKey[_KEY_NUMBER];			// Sorter Key, index of corresponding field in Field_enum
	bool				m_bLogo, m_bFirstLogo;			// Set if TA Logo present
	bool				m_bCustoLogo, m_bFirstCustoLogo;// HYS-1090: Set if Custo Logo is present
	bool				m_bFirstPage;						// First Page header/footer different
	double				m_ardMargin[_PAGESETUP_ENUM];
	TCHAR				m_artzText[_PAGESETUP_ENUM][_HEADER_LENGTH + 1];
	PageSetup_Alignment	m_peAlignment[_PAGESETUP_ENUM];
	LOGFONT				m_arLogFont[_PAGESETUP_ENUM];
	COLORREF			m_arFontColor[_PAGESETUP_ENUM];
	bool				m_bShowDate;
	bool				m_bShowUserRef;
	bool				m_bShowOtherInfo;
	bool				m_bShowPrjName;
	bool				m_bShowCustRef;
	bool				m_bShowComment;
	bool				m_bPrintPrjInfo;
};

class CDB_PageSetup : public CDB_PageField , protected PageSetup_struct
{
	DEFINE_DATA( CDB_PageSetup )

// Constructor and destructor.
protected:
	CDB_PageSetup( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_PageSetup() {}

// Public enumeration defines.
public:

	// WARNING order must be respected for enCheck

	// CDB_PAGESETUP_VERSION <= 5
	enum enCheckOld
	{
		oldBV,
		oldDPC,
		oldTRV,
		oldCV,
		oldHUB,
		oldPARTDEF,
		oldPIPELIST,
		oldARTLIST,
		oldDIVERSITYFACTOR,
		oldLASTFIELD
	};

	// CDB_PAGESETUP_VERSION = 16
	enum enCheck
	{
		FIRSTFIELD = 0,
		AIRVENTSEPARATOR = FIRSTFIELD,
		PRESSMAINT,
		SAFETYVALVE,
		BV,
		DPC,
		DPCBCV,
		CV,
		SMARTCONTROLVALVE,
		SMARTDPC,
		SIXWAYVALVE,
		SV,
		TRV,
		FLOORHEATINGCONTROL,
		TAPWATERCONTROL,
		HUB,
		PARTDEF,
		PIPELIST,
		DIVERSITYFACTOR,
		ARTLIST,
		TENDERTEXT,
		LASTFIELD
	};

// Public methods.
public:
	void SetName( TCHAR *tzName );
	void SetKey( int iKey, PageField_enum eField );
	void SetLogo( bool fLogo );
	void SetFirstLogo( bool fFirstLogo );
	// HYS-1090
	void SetCustoLogo( bool fLogo );
	void SetFirstCustoLogo( bool fFirstLogo );
	void SetFirstPageDifferent( bool fFirstPage );
	void SetMargin( PageSetup_enum ePage, double dMargin );
	void SetText( PageSetup_enum ePage, TCHAR *tzText );
	void SetLogFont( PageSetup_enum ePage, LOGFONT *pLogFont );
	void SetAlignment( PageSetup_enum ePage, PageSetup_Alignment eAlign );
	void SetFontColor( PageSetup_enum ePage, COLORREF color );

	void SetShowDate( bool bShowDate ) { if( m_bShowDate != bShowDate ) { m_bShowDate = bShowDate; Modified(); } }
	void SetShowUserRef( bool bShowUserRef ) { if( m_bShowUserRef != bShowUserRef ) { m_bShowUserRef = bShowUserRef; Modified(); } }
	void SetShowOtherInfo( bool bShowOtherInfo ) { if( m_bShowOtherInfo != bShowOtherInfo ) { m_bShowOtherInfo = bShowOtherInfo; Modified(); } }
	void SetShowPrjName( bool bShowPrjName ) { if( m_bShowPrjName != bShowPrjName ) { m_bShowPrjName = bShowPrjName; Modified(); } }
	void SetShowCustRef( bool bShowCustRef ) { if( m_bShowCustRef != bShowCustRef ) { m_bShowCustRef = bShowCustRef; Modified(); } }
	void SetShowComment( bool bShowComment ) { if( m_bShowComment != bShowComment ) { m_bShowComment = bShowComment; Modified(); } }
	void SetPrintPrjInfo( bool bPrintPrjInfo ) { if( m_bPrintPrjInfo != bPrintPrjInfo ) { m_bPrintPrjInfo = bPrintPrjInfo; Modified(); } }
	void SetCheck( enum enCheck part, bool bFlag ) { if( m_bCheck[part] != bFlag ) { m_bCheck[part] = bFlag; Modified(); } }
	void ResetCheck( bool bFlag = false ) { for( int i = enCheck::FIRSTFIELD; i < enCheck::LASTFIELD; i++ ) SetCheck( (enCheck)i, bFlag ); }

	LPCTSTR GetName() { return m_tzName; }
	PageField_enum GetKey( int iKey ) { ASSERT( iKey >= 0 && iKey < _KEY_NUMBER); return m_areKey[iKey]; }
	bool GetLogo() { return m_bLogo; }
	bool GetFirstLogo() { return m_bFirstLogo; }
	// HYS-1090
	bool GetCustoLogo() { return m_bCustoLogo; }
	bool GetFirstCustoLogo() { return m_bFirstCustoLogo; }
	bool GetFirstPageDifferent() { return m_bFirstPage; }
	double GetMargin( PageSetup_enum ePage ) { return m_ardMargin[ePage]; }
	LPCTSTR GetText( PageSetup_enum ePage);
	void GetLogFont( PageSetup_enum ePage, LOGFONT *pLogFont) { memcpy( pLogFont, &m_arLogFont[ePage], sizeof( LOGFONT ) ); }
	PageSetup_Alignment GetAlignment( PageSetup_enum ePage ) { return m_peAlignment[ePage]; }
	COLORREF GetFontColor(PageSetup_enum ePage ) { return m_arFontColor[ePage]; }
	bool GetShowDate() { return m_bShowDate; }
	bool GetShowUserRef() { return m_bShowUserRef; }
	bool GetShowOtherInfo() { return m_bShowOtherInfo; }
	bool GetShowPrjName() { return m_bShowPrjName; }
	bool GetShowCustRef() { return m_bShowCustRef; }
	bool GetShowComment() { return m_bShowComment; }
	bool GetPrintPrjInfo() { return m_bPrintPrjInfo; }
	bool GetCheck( enum enCheck part );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	bool m_bCheck[enCheck::LASTFIELD];
};
#endif	// TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_UserProduct
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_AlienProduct : public CDB_Thing
{
	DEFINE_DATA( CDB_AlienProduct )

protected:
	CDB_AlienProduct( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_AlienProduct() {}

// Public methods.
public:
	LPCTSTR GetName();
	IDPTR GetFamIDPtr();
	IDPTR GetSizeIDPtr();
	int GetSizeKey();

	void SetName( LPCTSTR name ) { m_Name=name; }
	
 // Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	virtual CheckFilterReturnValue CheckFilter( CFilterTab *pFilter, CString filter );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	_string m_Name;
	IDPTR	m_FamIDPtr;
	IDPTR	m_SizeIDPtr;
};

class CDB_AlienRegValve : public CDB_AlienProduct
{
	DEFINE_DATA( CDB_AlienRegValve )

protected:
	CDB_AlienRegValve( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_AlienRegValve() {}

// Public methods.
public:
	IDPTR GetValveCharIDPtr();
	CDB_ValveCharacteristic *GetValveCharacteristic(); 

	double GetKvSignal() const { return m_dKvsSignal; }
	bool IsKvSignalEquipped() { return (m_dKvsSignal > 0); }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_ValveCharIDPtr;
	double m_dKvsSignal;
};


////////////////////////////////////////////////////////////////
//
//		CRegValvAlienTab :
//
////////////////////////////////////////////////////////////////
//	Inherit from CTable
////////////////////////////////////////////////////////////////
class CRegValvAlienTab : public CTable
{
	DEFINE_DATA( CRegValvAlienTab )

public:
	CRegValvAlienTab( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CRegValvAlienTab() {}
};


////////////////////////////////////////////////////////////////
//
//		CFamAlienTab :
//
////////////////////////////////////////////////////////////////
//	Inherit from CTable
////////////////////////////////////////////////////////////////
class CFamAlienTab : public CTable
{
	DEFINE_DATA( CFamAlienTab )
public:
	CFamAlienTab( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CFamAlienTab() {}
};


////////////////////////////////////////////////////////////////
//
//		CCharAlienTab :
//
////////////////////////////////////////////////////////////////
//	Inherit from CTable
////////////////////////////////////////////////////////////////
class CCharAlienTab : public CTable
{
	DEFINE_DATA( CCharAlienTab )
public:
	CCharAlienTab( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CCharAlienTab() {}
};


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_RadNomCond
//
//	new:CDB_RadNomCond(ID)->tableID	Common CData syntax
//	{
//		Ts
//		Tr
//		Ti
//	}
//
///////////////////////////////////////////////////////////////////////////////////

struct RadNomCond_struct{
	double m_dTs;
	double m_dTr;
	double m_dTi;
	bool m_bFix;
};

class CDB_RadNomCond : public CData
{
	DEFINE_DATA( CDB_RadNomCond )

protected:
	CDB_RadNomCond( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_RadNomCond() {}

// Public methods.
public:
	LPCTSTR GetName();

	double GetTs() { return m_RadNomCond_struct.m_dTs; }
	double GetTr() { return m_RadNomCond_struct.m_dTr; }
	double GetTi() { return m_RadNomCond_struct.m_dTi; }
	bool IsFix() { return m_RadNomCond_struct.m_bFix; }				// Used to protect a TADB information

	void SetTs( double dTs ) { m_RadNomCond_struct.m_dTs = dTs; Modified(); }
	void SetTr( double dTr ) { m_RadNomCond_struct.m_dTr = dTr; Modified(); }
	void SetTi( double dTi ) { m_RadNomCond_struct.m_dTi = dTi; Modified(); }
	void SetFix( bool bFlag ) { m_RadNomCond_struct.m_bFix = bFlag; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

protected:

	/////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	/////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	RadNomCond_struct m_RadNomCond_struct;
};	
#endif	//TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_CircSchemeCateg
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_CircSchemeCateg : public CData
{
	DEFINE_DATA( CDB_CircSchemeCateg )

protected:
	CDB_CircSchemeCateg( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_CircSchemeCateg() {}

// Public enumeration defines.
public:
	enum eFLOWVAR
	{
		Undef = -1,
		Constant,
		Variable,
		LastFLOWVAR
	};
	
	enum e3WType
	{
		e3wTypeUndef = 0,
		e3wTypeDividing,
		e3wTypeMixing,
		e3wTypeMixDecByp,
		e3wTypeDoubleMix,
		e3wTypeLastType
	};

// Public methods.
public:	
	_string GetSchName();
	_string GetAbbrSchName();
	int GetOrderNr() { return m_iOrderNr; } 
	eFLOWVAR GetVarPrimFlow() { return (eFLOWVAR)m_iVarPrimFlow; }
	eFLOWVAR GetVarSecondFlow() { return (eFLOWVAR)m_iVarSecondFlow; }
	bool Is3W() { return ( CDB_ControlProperties::CV3W == m_e2W3W ); }
	bool Is2W() { return ( CDB_ControlProperties::CV2W == m_e2W3W ); }
	bool Is6W() { return ( CDB_ControlProperties::CV6W == m_e2W3W ); }
	bool IsInjection() { return m_bInjection; }
	bool IsDirectCircuit() { return (0 == IDcmp( _T("SCHCATEG_DC"),GetIDPtr().ID ) ); }
	bool IsStraight() { return (0 == IDcmp( _T("SCHCATEG_SP"),GetIDPtr().ID ) ); }
	bool IsPending() { return (0 == IDcmp( _T("SCHCATEG_PDG"),GetIDPtr().ID ) ); }
	bool IsPump() { return (0 == IDcmp( _T("SCHCATEG_PUMP"),GetIDPtr().ID ) ); }

	bool IsSecondarySideExist(){return (IsInjection());}

	bool IsActiveCircuit() { return ( IsInjection()||IsPump() ); }

	CDB_ControlProperties::CV2W3W Get2W3W() { return m_e2W3W; }
	e3WType Get3WType() const { return m_e3WType; }

protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

#ifndef TACBX 
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	_string m_SchCategName;
	_string m_AbbrSchCategName;
	int m_iOrderNr;									// Ordinal number of the circuit scheme category
	int m_iVarPrimFlow;								// Primary side is -1: Undefined / 0: Constant flow / 1: Variable flow 
	int m_iVarSecondFlow;							// Secondary side is -1: Undefined / 0: Constant flow / 1: Variable flow 
	CDB_ControlProperties::CV2W3W m_e2W3W;			// -1: not used / 0: control circuit with 2-way valve / 1: control circuit with 3-way valve
	bool m_bInjection;								// true/false
	CDB_CircSchemeCateg::e3WType m_e3WType;			// Type of 3w control valve
	_string *m_pStr;
};


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_MultiPair
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_MultiPair : public CData
{
	DEFINE_DATA( CDB_MultiPair )

protected:
	CDB_MultiPair( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_MultiPair() {}

// Public methods.
public:
	bool GetPair( unsigned int index, std::pair<int,int> &pt );
	bool GetPoint( unsigned int index, CPoint &pt );
	int GetSize() { return m_List.size(); }

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	std::vector<std::pair<int,int> > m_List;
};
#endif	// TACBX


///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_CircuitScheme
//
//	new:CDB_CircuitScheme(ID)->tableID	Common CData syntax
//	{
//		SchemeCategID					ID of the scheme category the scheme belongs to
//		"Scheme Name"					Displayed scheme name
//		ResourceImgID					Resource ID of the scheme in the HydronicPics DLL
//		m_BalType;						Enum - eBALTYPE_SP( Straight Pipe), eBALTYPE_BV, eBALTYPE_DPC or eBALTYPE_ELECTRONIC.
//		m_DpCType;						Enum - eDPCTYPE_NU, eDPCTYPE_DPC, eDPCTYPE_PICV, eDPCTYPE_DPCBCV or eDPCTYPE_SMARTDP.
//		m_TermUnit;						Enum - eTERMUNIT_NotAllowed, eTERMUNIT_Allowed or eTERMUNIT_Compulsory.
//		m_CVType;						Enum - eCVFUNC_NoControl, eCVFUNC_ControlOnly, eCVFUNC_Presettable, eCVFUNC_PresetPT or eCVFUNC_Electronic.
//		m_BoxesID						Box description ID
//		m_eDpStab						Enum - eDpStab_Branch, eDpStab_Cv or eDpStab_Cv.
//		m_eMvLoc						Enum - eMvLoc_Prim, eMvLoc_Sec or eMvLoc_None.
//		m_eDpCLoc						Enum - eDpCLoc_DownStream, eDpCLoc_UpStream or eDpCLoc_None.
//		m_bSet							Byte: 0: scheme is not for a set; 1: scheme is for a set.
//	    m_arAnchorPt					List of anchoring points coma separated
//										CAnchorPt::eFunc - NU,BV_P,BV_Byp,BV_S,ControlValve,DpC,PICV,Pump,ShutoffValve,DPCBCV, SmartControlValve 
//										if first char is '*' AnchorPt is optional.
//										HYS-1716: Each anchoring point definition is now followed by [xxx]
//											1st position: Pipe type 		-> 'd' (For 'Distribution'), 'c' (For 'Circuit') or 'n' (For 'Not applicable')
//											2nd position: Pipe locatization -> 's' (For 'Supply'), 'r' (For 'Return'), 'b' (For 'Bypass') or 'n' (For 'Not applicable')
//											3rd position: Circuit side		-> 'p' (For 'Primary'), 's' (For 'Secondary) or 'n' (For 'Not applicable')
//	}
///////////////////////////////////////////////////////////////////////////////////
class CDB_CircuitScheme : public CData
{
	DEFINE_DATA( CDB_CircuitScheme )

// Public enumeration defines.
public:
	enum eBALTYPE
	{
		SP,					// Straight pipe
		BV,					// Manual balancing 
		DPC,				// Dp control
		ELECTRONIC,			// Electronic balancing
		ELECTRONICDPC,		// Electronic Dp control
		LastBALTYPE
	};

	enum eDpCType
	{
		eDpCTypeNU,			// Not used	
		eDpCTypeDPC,		// Pure DpC
		eDpCTypePICV,		// PICV 
		eDpCTypeDPCBCV,		// For example: TA-COMPACT-DP
		eDpCTypeSMARTDP,	// For example: TA-Smart Dp
		eDpCTypeLast
	};
	
	enum eTERMUNIT
	{
		NotAllowed,
		Allowed,
		Compulsory,
		LastTERMUNIT
	};
	
	enum eBoxes
	{
		ebNoBox = -1,
		ebPrimaryValues,
		ebCircuitPrimaryPipe,
		ebDistributionPipe,
		ebTU,
		ebCV,
		ebSmartControlValve,
		ebBVP,
		ebBVB,
		ebDPC,
		ebRem,
		ebPump,
		ebBVS,
		ebCircuitSecondaryPipe,
		ebSecondaryValues,
		ebDistributionReturnPipe,				// Pipe distribution in reverse mode.
		ebShutoffValve,
		ebDpCBCValve,
		ebSmartDpC,
		ebCircuitByPassPipe,
		LasteBox
	};

protected:
	CDB_CircuitScheme( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_CircuitScheme() {}

// Public methods.
public:
	LPCTSTR GetSchemeCategID() const { ASSERT (*m_SchemeCategIDPtr.ID != '\0'); return m_SchemeCategIDPtr.ID; };
	LPCTSTR GetBoxesID() const { ASSERT (*m_BoxesIDPtr.ID != '\0'); return m_BoxesIDPtr.ID; };

	IDPTR GetSchemeCategIDPtr();
	CDB_CircSchemeCateg *GetpSchCateg();
	IDPTR GetBoxesIDPtr();

	_string	GetSchemeName();
	_string	GetSchemeCategName();
	_string	GetAbbrevSchCategName();

	_string GetDynCircuitID() { return m_DynCirScheme; }
	
	eBALTYPE GetBalType() { return (eBALTYPE)m_iBalType; }
	_string GetBalTypeStr();
	static _string GetBalTypeStr( eBALTYPE BalType );
	eDpCType GetDpCType() { return (eDpCType)m_iDpCType; }
	eTERMUNIT GetTermUnit() { return (eTERMUNIT)m_iTermUnit; }
	CDB_ControlProperties::eCVFUNC GetCvFunc() { return ( (CDB_ControlProperties::eCVFUNC)m_iCVFunc ); }

	eDpStab GetDpStab() { return m_eDpStab; }
	eMvLoc GetMvLoc() { return m_eMvLoc; }
	eDpCLoc GetDpCLoc() { return m_eDpCLoc; }
	CvLocation GetCvLoc();
	ShutoffValveLoc GetShutoffValveLoc();
	SmartValveLocalization GetSmartControlValveLoc();
	SmartValveLocalization GetSmartDpCLoc();

	bool IsForASet() { return m_bSet; }

#ifndef TACBX 
	CPoint GetBox(eBoxes box);
	
	// Return background color of anchoring point.
	COLORREF GetAnchorPtColor( CAnchorPt::eFunc func );
	
	// Set anchoring point color.
	void SetAnchorPtColor ( CAnchorPt::eFunc func, COLORREF col );
#endif

	// 2017-01-16: These methods are now available in TA-Scope.
	int GetAnchorPtListSize() { return (int)m_mapAnchorPt.size(); }
	CAnchorPt::eFunc GetAnchorPtFunc( int index );
	bool IsAnchorPtOptional( CAnchorPt::eFunc func );
	
	// Check is the func exist into the current list of Anchoring points.
	bool IsAnchorPtExist( CAnchorPt::eFunc func );

	CAnchorPt::AnchorPtFunc *GetAnchorPtDetails( CAnchorPt::eFunc func );
	CAnchorPt::PipeType GetAnchorPtPipeType( CAnchorPt::eFunc func );
	CAnchorPt::PipeLocation GetAnchorPtPipeLocation( CAnchorPt::eFunc func );
	CAnchorPt::CircuitSide GetAnchorPtCircuitSide( CAnchorPt::eFunc func );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
#ifndef TACBX
	virtual void CrossVerifyID();
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual bool Read( INPSTREAM inpf );
	virtual void Write( OUTSTREAM outf );

#ifndef TACBX
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
#endif
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_SchemeCategIDPtr;											// Scheme category IDPTR
	_string	m_SchemeName;												// Displayed scheme name
	_string	m_DynCirScheme;												// Circuit scheme Name into the Circuit scheme DLL
	int		m_iBalType;													// 0: SP (Straight Pipe) / 1: BV / 2: DpC
	int		m_iDpCType;													// 0: DpC / 1: PICV
	int		m_iTermUnit;												// 0: Terminal Unit not allowed / 1: Terminal Unit allowed / 2: Terminal unit compulsory 
	int		m_iCVFunc;													// 0: No control / 1: Control only / 2: Presettable / 3: Presettable with meas. points
	bool	m_bBVin3WBypAllowed;										// 0: No BV in bypass of the 3W control valve																															
	IDPTR	m_BoxesIDPtr;												// IDPTR of the corresponding Boxes
	eDpStab	m_eDpStab;													// 0 Branch, 1 Control valve
	eMvLoc	m_eMvLoc;													// 0 Primary, 1 Secondary
	eDpCLoc	m_eDpCLoc;													// 0 DownStream, 1 UpStream
	CvLocation m_eCvLoc;												// 0 Primary side, 1 Secondary side
	ShutoffValveLoc m_eShutoffValveLoc;									// 0 Primary side, 1 Secondary side
	SmartValveLocalization m_eSmartControlValveLoc;						// 0 Supply, 1 Return
	SmartValveLocalization m_eSmartDpCLoc;	// 0 Supply, 1 Return
	bool m_bSet;

	std::map<CAnchorPt::eFunc, CAnchorPt::AnchorPtFunc> m_mapAnchorPt;		// Map of anchoring points used to build hydronic scheme.

#ifndef TACBX 
	std::map<unsigned short, COLORREF> m_mapAnchorPtBckColor;
#endif
};


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Singularity
//
///////////////////////////////////////////////////////////////////////////////////

class CDB_Singularity : public CData
{
	DEFINE_DATA( CDB_Singularity )

// Public enumeration defines.
public:
	enum eXType
	{
		Constant, 
		Va_Vb,  
		Vb_Vbp, 
		Vbp_Vb, 
		Sa_Sb,
		Last
	};
		
// Constructor and destructor.
protected:
	CDB_Singularity( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Singularity() { delete m_pDzetaChar; };

// Public methods.
public:
	CString GetDescription();
	
	// Check if characteristic is discrete.
	eXType GetXType() { return m_XType;};
	
	// Get the maximum Dzeta.
	double GetDzetaMax() { return m_pDzetaChar->GetMaxFX(); }
	
	// Get the minimum Dzeta.
	double GetDzetaMin() { return m_pDzetaChar->GetMinFX(); }
	
	// Get the Dzeta value from x value.
	double GetDzeta( double x );
	
	// Get the maximum X.
	double GetXMax() { return m_pDzetaChar->GetMaxX(); }
	
	// Get the minimum X.
	double GetXMin() { return m_pDzetaChar->GetMinX(); }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	int m_nData;
	_string m_Description;			// Displayed description
	eXType m_XType;					// Define x value va/vb, sa/sb
	CCurveFitter *m_pDzetaChar;		// Pointer on Kv-position characteristic(s)
};		
#endif	//TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_HubValv
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_HubValv : public CData
{
	DEFINE_DATA( CDB_HubValv )

// Public enumeration defines.
public:
	typedef enum eSupplyReturnValv
	{
		Supply	=1,
		Return	=2,
		Last	=3
	};

// Constructor and destructor.
protected:
	CDB_HubValv( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_HubValv() {}

// Public methods.
public:
	eSupplyReturnValv GetSupplyOrReturn() { return m_eSupRetValv; }
	CString GetName();
	IDPTR GetValveIDPtr();
	CString GetTAISPartner() { return m_TAISHubPartner; }
	CString GetTAISSize() { return m_TAISHubPartnerSize; }
	CString GetTAISDPRange() { return m_TAISPartnerDP; }
	UINT GetImageID() { return m_ImageID; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	///////////////////////////////////////////////////////////////////////////////////
	
// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );
	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	eSupplyReturnValv m_eSupRetValv;
	_string m_Name;
	IDPTR m_IDPtrValve;
	CString m_TAISHubPartner;
	CString m_TAISHubPartnerSize;
	CString m_TAISPartnerDP;
	UINT m_ImageID;
};		
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_HubSupRetLnk
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_HubSupRetLnk : public CData
{
	DEFINE_DATA( CDB_HubSupRetLnk )

// Constructor and destructor.
protected:
	CDB_HubSupRetLnk( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_HubSupRetLnk() {}

// Public methods.
public:
	IDPTR GetValveTypeIDPtr();
	CString GetValveTypeID() { return GetValveTypeIDPtr().ID; }
	CString GetTAISSupply() { return m_TAISHubPartnerSupply; }
	CString GetTAISReturn() { return m_TAISHubPartnerReturn; }
	static IDPTR FindMatchingPairs( CString TAISSupplyID, CString TAISReturnID );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Called by Database object before its destruction
	virtual void OnDatabaseDestroy();
	
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	IDPTR m_IDPtrValvType;
	CString m_TAISHubPartnerSupply;
	CString m_TAISHubPartnerReturn;	
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_HubStaValv
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_HubStaValv : public CData
{
	DEFINE_DATA( CDB_HubStaValv )

// Public enumeration defines.
public:
	enum eSupplyReturnValv
	{
		Supply	=1,
		Return	=2,
		Last	=3
	};

// Constructor and destructor.
protected:
	CDB_HubStaValv( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_HubStaValv() {}

// Public methods.
public:
	eSupplyReturnValv GetSupplyOrReturn() { return m_eSupRetValv; }
	CString GetName();
	IDPTR GetValveIDPtr();
	CString GetTAISValve() { return m_TAISValve; }
	CString GetTAISSize() { return m_TAISSize; }
	IDPTR GetActuatorIDPtr() { return m_IDPtrActuator; }
	UINT GetImageID() { return m_ImageID; }

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	///////////////////////////////////////////////////////////////////////////////////
	
// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	//		Supply/Return
	//		"Name"		
	//		ID on valve char					(TA400==NO_ID)
	//		"TAIS_HUB_SUPPLY/RETURN"			(PLUG; TA400...)
	//		"TAIS_HUB_SIZE"						(DN15...)
	//		ID on Hub supply actuator (table?)	(can be NO_ID)
	//		Image identificator		

	eSupplyReturnValv m_eSupRetValv;
	_string m_Name;
	IDPTR m_IDPtrValve;
	CString m_TAISValve;
	CString m_TAISSize;
	IDPTR m_IDPtrActuator;
	UINT m_ImageID;
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_Currency
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_Currency : public CData
{
//	new:CDB_Currency(ID) -> CURRENCY_TAB
//	{
//		CString	IsoCurrency;
//		CString	TAISCurrency;
//		CString	IDS_Country;
//		int 	Index;
//	}
	DEFINE_DATA( CDB_Currency )

// Constructor and destructor.
protected:
	CDB_Currency( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_Currency() {}

// Public methods.
public:
	_string	GetISO() { return m_ISO.c_str(); }
	CString GetTAIS() { return m_TAIS; }
	_string GetName();
	int GetIndex() { return m_Index; }

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount);
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	_string	m_ISO;
	// We keep CString for m_TAIS because _string m_TAIS is in conflict with Zlib in DlgExportConfigFile
	CString m_TAIS;
	_string m_Name;
	int m_Index;
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_PriceList
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_PriceList : public CData
{
	DEFINE_DATA( CDB_PriceList )

protected:
	CDB_PriceList( CDataBase *pDataBase, LPCTSTR ID );
	~CDB_PriceList() {}
	
// Public methods.
public:
	double GetPrice( CString strArticleNumber );

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	void _CleanArticleNumber( CString &strArticleNumber );

// Private variables.
private:
	std::map<CString, double> m_mapPriceList;
};
#endif	// TACBX


#ifndef TACBX
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_MapStrStr
//
///////////////////////////////////////////////////////////////////////////////////
class CDB_MapStrStr : public CData
{
	DEFINE_DATA( CDB_MapStrStr )

// Public methods.
public:
	CDB_MapStrStr( CDataBase *pDataBase, LPCTSTR ID );
	void Add( CString strFirst, CString strSecond );
	CString Get( CString strFirst );

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM  inpf );
	///////////////////////////////////////////////////////////////////////////////////

// Private variables.
private:
	CTemplateMap<CString, CString>	m_mapStrStr;
};
#endif	//TACBX


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_StringArray
//
//	Inherited from CData
//
//	{
//		n							        // Number of column of the selection: int format
//		str1 \ str2 .. \ strn				// array of string contains data in string format
//		str'1 \ str'2 .. \ str'n
//		...
//	}
///////////////////////////////////////////////////////////////////////////////////
class CDB_StringArray : public CData
{
	DEFINE_DATA( CDB_StringArray )

protected:
	CDB_StringArray( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_StringArray();
	
// Public methods.
public:
	int GetNcol() { return m_nCol;}
	void SetNcol(int iNcol) { m_nCol = iNcol; }
	void GetStringTabs(vector<vector<CString>> ** pVectStringTab);

	// SETTER.
	void SetCol( int nCol ) { m_nCol = nCol; }
	void SetStringTabs( vector<vector<CString>> *pVecStringTabs );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

	void _Reset();

protected:
	int m_nCol;
	vector<vector<CString>> *m_pVecStringTabs;
};
#endif	//TACBX


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_BufferVesselTI
//
//	Inherited from CDB_StringArray
//
//	{
//		n							        // Number of column of the selection.
//		strID1 \ str1 \ str2 .. \ strn		// array of string contains data in strng format
//		strID2 \ str'1 \ str'2 .. \ str'n
//		...
//	}
//  Note: - Parent class CDB_StringArray str contains "<value" or ">value"
//        - value is saved in m_vectVDList
//        - strn contains Buffer Vessel ID
///////////////////////////////////////////////////////////////////////////////////
class CDB_BufferVesselTI : public CDB_StringArray
{
	DEFINE_DATA( CDB_BufferVesselTI )

protected:
	CDB_BufferVesselTI(CDataBase *pDataBase, LPCTSTR ID);
	virtual ~CDB_BufferVesselTI();

// Public methods.
public:
	vector<std::pair<CString, double>> GetVectVDList() { return m_vectVDList; }
	vector<std::pair<CString, double>> GetVectPSVList() { return m_vectPSVList; }
	vector<CString> *GetStringList() { return m_pVectStringList; }

	void SetStringList( vector<CString> *pListOfValues ) { m_pVectStringList = pListOfValues; }
	void Reset();
	void CopyAllVectorsFrom( CDB_BufferVesselTI *pclSourceBufferVesselTI );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	// Disk IO.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

// Protected variables.
protected:
	vector<std::pair<CString, double>> m_vectVDList;
	vector<std::pair<CString, double>> m_vectPSVList;
	vector<CString> *m_pVectStringList;
};
#endif	// TACBX


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_BlowTankAllocation
//
//	Inherited from CDB_StringArray
//
//	{
//		6							        // Number of column of the selection.
//		SafetyValveFamilyID1 \ SizeID \ psv limit 1 \ BlowTankID1 \ psv limit 2 \ BlowTankID2
//		SafetyValveFamilyID2 \ SizeID \ psv limit 1 \ BlowTankID3 \ psv limit 2 \ BlowTankID4
//		...
//	}
//  Note: - values are saved in 'm_mapBlowTankAllocation'.
//        - "psv limit" must be preceded by '<', '<=', '>' or '>='.
///////////////////////////////////////////////////////////////////////////////////
class CDB_BlowTankAllocation : public CDB_StringArray
{
	DEFINE_DATA( CDB_BlowTankAllocation )

protected:
	CDB_BlowTankAllocation( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_BlowTankAllocation() {}

// Public some definitionss.
public:
	// Respect the same order as the one in the 'ReadText' function:
	// _T("<="), _T(">="), _T("<"), _T(">").
	enum OperatorSign
	{
		Undefined = -1,
		LowerOrEqualThan = 0,
		GreaterOrEqualThan = 1,
		LowerThan = 2,
		GreaterThan = 3
	};

	struct BTAllocData
	{
		int m_iOperatorSign;
		double m_dPSVLimit;
		CString m_strBlowTankID;
	};

	typedef std::map<CString, std::map<CString, std::vector<BTAllocData>>> mapBlowTankAllocation;

// Public methods.
public:
	mapBlowTankAllocation &GetBlowTankAllocation() { return m_mapBlowTankAllocation; }
	CString GetBlowTank( CDB_SafetyValveBase *pclSafetyValve, double dPSV );

	// SETTER.
	void CopyBlowTankAllocationFrom( CDB_BlowTankAllocation *pclSourceBlowTankAllocation );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////

private:
	mapBlowTankAllocation m_mapBlowTankAllocation;
};
#endif	// TACBX


#ifndef TACBX 
///////////////////////////////////////////////////////////////////////////////////
//
//								CDB_6WayValveActuatorCompatibility
//
//	Inherited from CDB_StringArray
//
//	{
//		n							        // Number of column of the selection.
//		str1 \ str2 \ str3 .. \ strn		// array of string contains data in strng format
//		str'1 \ str'2 \ str'3 .. \ str'n
//		...
//	}
//
//  n = 4
//   str1 = Enum that describes the type of circuit used with the 6-way valve.
//   str2 = Power supply ID list for actuator compatible with 6-way valve.
//     remark: You can put more that one power supply ID with '|' as separator.
//   str3 = Input signal ID list for actuator compatible with 6-way valve.
//     remark: You can put more that one input signal ID with '|' as separator.
//   str4 = Family ID for actuator compatibles with PIBCV valve.
///////////////////////////////////////////////////////////////////////////////////
class CDB_6WayValveActuatorCompatibility : public CDB_StringArray
{
	DEFINE_DATA( CDB_6WayValveActuatorCompatibility )

protected:
	CDB_6WayValveActuatorCompatibility( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CDB_6WayValveActuatorCompatibility() {}

// Public structure defines.
public:
	struct ActuatorFilters
	{
		std::vector<IDPTR> m_6WayValveActuatorPowerSupplyIDCompatibilityList;
		std::vector<IDPTR> m_6WayValveActuatorInputSignalIDCompatibilityList;
		std::vector<IDPTR>  m_PIBCValveActuatorFamilyIDCompatibilityList;
	};

// Public methods.
public:	
	ActuatorFilters *GetOneActutorFilters( e6WayValveSelectionMode eSixWayAppType );
	std::map<e6WayValveSelectionMode, ActuatorFilters> &GetAllActuatorFilters() { return m_mapActuatorFilters; }

	// SETTER.
	void CopyAllActuatorFilters( CDB_6WayValveActuatorCompatibility *pclFrom6WayValveActuatorCompatibility );

	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' public virtual method.
	virtual void CrossVerifyID();
	virtual void Copy( CData *pclDestination );
	///////////////////////////////////////////////////////////////////////////////////

// Protected methods.
protected:
	///////////////////////////////////////////////////////////////////////////////////
	// Overrides the 'CData' protected virtual method.
	virtual void Write( OUTSTREAM outf );
	virtual bool Read( INPSTREAM inpf );

	virtual void ReadText( INPSTREAM inpf, unsigned short *pusLineCount );
	///////////////////////////////////////////////////////////////////////////////////s

private:
	std::map<e6WayValveSelectionMode, ActuatorFilters> m_mapActuatorFilters;
};
#endif	// TACBX


class CTableDN : public CTable
{
	DEFINE_DATA( CTableDN )

protected:
	CTableDN( CDataBase *pDataBase, LPCTSTR ID );
	virtual ~CTableDN() {}

// Public methods.
public:
	int GetLowestDN();
	int GetBiggestDN();
	int GetSize( _string strID );

// Private variables.
private:
	int m_iLowestDN;
	int m_iBiggestDN;
};

#endif // !defined(DATABOBJ_H__INCLUDED_)
