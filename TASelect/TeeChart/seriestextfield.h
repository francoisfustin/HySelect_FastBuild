#if !defined(AFX_SERIESTEXTFIELD_H__F993DD2D_0A3B_41A9_A8E0_A991634A31DD__INCLUDED_)
#define AFX_SERIESTEXTFIELD_H__F993DD2D_0A3B_41A9_A8E0_A991634A31DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Machine generated IDispatch wrapper class(es) created by Microsoft Visual C++

// NOTE: Do not modify the contents of this file.  If this class is regenerated by
//  Microsoft Visual C++, your modifications will be overwritten.

/////////////////////////////////////////////////////////////////////////////
// CSeriesTextField wrapper class

class CSeriesTextField : public COleDispatchDriver
{
public:
	CSeriesTextField() {}		// Calls COleDispatchDriver default constructor
	CSeriesTextField(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CSeriesTextField(const CSeriesTextField& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	long GetFieldIndex();
	void SetFieldIndex(long nNewValue);
	CString GetFieldName();
	void SetFieldName(LPCTSTR lpszNewValue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIESTEXTFIELD_H__F993DD2D_0A3B_41A9_A8E0_A991634A31DD__INCLUDED_)
