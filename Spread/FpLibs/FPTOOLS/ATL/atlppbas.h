/****************************************************************************
* ATLPPBAS.H  -  Declaration of FarPoint's CfpAtlPropPageBase class
*
* Copyright (C) 1991-1998 - FarPoint Technologies
* All Rights Reserved.
*
* No part of this source code may be copied, modified or
* reproduced in any form without retaining the above
* copyright notice.  This source code, or source code
* derived from it, may not be redistributed without
* express written permission of FarPoint Technologies.
*
****************************************************************************/
#ifndef __FPATLPROPPAGEBASE_H_
#define __FPATLPROPPAGEBASE_H_

class CfpAtlPropPageBase
{
//	This class provides some static helper functions for data 
//	transfer between dialog controls and your control's 
//	dispatch interface.  They perform type conversions for you.
//	You must synchronize access to the functions in this class
//	with the CRITICAL_SECTION cs.
protected:
	static CRITICAL_SECTION cs;
	static int				nRefs;

public:
	CfpAtlPropPageBase::CfpAtlPropPageBase()
	{	// Initialize critical section the first time in
		if (!nRefs++)
			InitializeCriticalSection(&cs);
	}

	CfpAtlPropPageBase::~CfpAtlPropPageBase()
	{	// Delete critical section the last time out
		if (!--nRefs)
			DeleteCriticalSection(&cs);
	}

//	Puts an enum property value into a ComboBox control.
//	Cast your enum to a short and pass it into nVal.
	BOOL PutDlgComboEnum(HWND hWndDlg, int nIDDlgCtrl, short nVal);

//	Gets and enum property value from a ComboBox control.
//	Case the return value to the appropriate enum type.
	short GetDlgComboEnum(HWND hWndDlg, int nIDDlgCtrl);

//	Puts an int property value into an Edit control. 
	BOOL PutDlgEditInt(HWND hWndDlg, int nIDDlgCtrl, int iVal, BOOL fSigned = TRUE);

//	Gets an int property value from an Edit control.
//	Pass in a pointer for lpTrans for error info
//  (if *lpTrans is FALSE after this function returns,
//	then an error occured during the call).
	int GetDlgEditInt(HWND hWndDlg, int nIDDlgCtrl, BOOL* lpTrans = NULL, BOOL fSigned = TRUE);

//	Puts a long property value into an Edit control. 
	BOOL PutDlgEditLong(HWND hWndDlg, int nIDDlgCtrl, long lVal);

//	Gets a long property value from an Edit control.
	long GetDlgEditLong(HWND hWndDlg, int nIDDlgCtrl);

//	Puts a float property value into an Edit control.
	BOOL PutDlgEditFloat(HWND hWndDlg, int nIDDlgCtrl, float flVal);

//	Gets a float property value from an Edit contorl.
	float GetDlgEditFloat(HWND hWndDlg, int nIDDlgCtrl);

//	Puts a BSTR property value into an edit control
//	Pass TRUE for fAutoRelease if you want the function to
//	free the BSTR passed to it before returning.
	BOOL PutDlgEditBstr(HWND hWndDlg, int nIDDlgCtrl, BSTR bstr, BOOL fAutoRelease = TRUE);

//	Gets a BSTR property value from an edit control
//	YOU MUST CALL SysFreeString() ON THIS BSTR AFTER YOU ARE DONE WITH IT!
	BSTR GetDlgEditBstr(HWND hWndDlg, int nIDDlgCtrl);

//	Puts a boolean property value into a CheckBox control.
	BOOL PutDlgCheckBoxBool(HWND hWndDlg, int nIDDlgCtrl, BOOL bVal);

//	Gets a boolean property value from a CheckBox Control.
	VARIANT_BOOL GetDlgCheckBoxBool(HWND hWndDlg, int nIDDlgCtrl);
};
#endif //__FPATLPROPPAGEBASE_H_

