
#pragma once

#include "Units.h"

// Class CNumString used to format the string correctly
// with or without units. Take into account a WM_UNIT_CHANGE
// to refresh the values and the units if they are shown
// It will work only for CEdit and CRichEdit box

class CNumString
{
public:
	CNumString();

	enum eEditType
	{
		eAll,
		eINT,
		eDouble
	};

	enum eEditSign
	{
		eBoth,
		ePositive,
	};

	// Set min and max value only for a double.
	void SetMinDblValue( double dMinVal )
	{
		m_dMinDblValue = dMinVal;
	}

	void SetMaxDblValue( double dMaxVal )
	{
		m_dMaxDblValue = dMaxVal;
	}

	// Set min and max value only for a int.
	void SetMinIntValue( int iMinVal )
	{
		m_iMinIntValue = iMinVal;
	}
	
	void SetMaxIntValue( int iMaxVal )
	{
		m_iMaxIntValue = iMaxVal;
	}

	void SetEditType( eEditType EdiType = eDouble, eEditSign EditSign = eBoth )
	{
		m_EditType = EdiType;
		m_EditSign = EditSign;
	}

	eEditType GetEditType()
	{
		return m_EditType;
	}

	void SetPhysicalType( ePHYSTYPE eType )
	{
		m_ePhysType = eType;
	}

	void SetPhysicalTypeString( CString strPhysType )
	{
		m_ePhysType = _U_NODIM;
		m_bPhysTypeStr = true;
		m_PhysTypeStr = strPhysType;
	}

	ePHYSTYPE GetPhysicalType()
	{
		return m_ePhysType;
	}

	void SetEditSign( eEditSign eSign )
	{
		m_EditSign = eSign;
	}

	void SetCurrentValSI( double ValSI )
	{
		m_dCurrentValSI = ValSI;
	}
	
	double GetCurrentValSI()
	{
		return m_dCurrentValSI;
	}
	
	void SetUnitsUsed( bool bUnitUsed = true )
	{
		m_bUnitsUsed = bUnitUsed;
	}

	// Return a pointer to the string
	// with updated units value
	LPCTSTR GetCurrentString();

	CString Update();

// Protected methods.
protected:
	// Parameters :
	// nChar = Input character
	// OldStr = String before input char
	// NewStr = String after have inserted char
	// iPosCursor = Position of the cursor after
	// the char is inserted
	// Return the string to be written
	CString FormatChar( UINT uiChar, CString strOldStr, CString strNewStr, int *piPosCursor );

	bool IsUnitsUsed()
	{
		return m_bUnitsUsed;
	}

// Protected variables.
protected:
	CNumString::eEditType m_EditType;
	CNumString::eEditSign m_EditSign;
	ePHYSTYPE m_ePhysType;
	CString m_PhysTypeStr;
	bool m_bPhysTypeStr;
	bool m_bUnitsUsed;
	double m_dMinDblValue;
	double m_dMaxDblValue;
	double m_dCurrentValSI;
	int m_iMinIntValue;
	int	m_iMaxIntValue;
};

// Class CCbiRestrString used to format the string correctly
// for the project name
// It will work only for CEdit and CRichEdit box

class CCbiRestrString
{
public:
	CCbiRestrString();

	bool CheckCBICharSet( CString *pstr );

protected:
	bool CheckCBICharSet( TCHAR zechar );
};
