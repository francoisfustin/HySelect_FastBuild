/****************************************************************************
* PPGeneral.cpp : Implementation of CPPGeneral
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
/////////////////////////////////////////////////////////////////////////////
//	History:
//	1.  08/28/98	SCL Created
// 
#include "stdafx.h"
#include "PPGeneral.h"

/////////////////////////////////////////////////////////////////////////////
// CPPGeneral
CPPGeneral::CPPGeneral() 
{
	m_dwTitleID = IDS_TITLEPPGeneral;
	m_dwHelpFileID = IDS_HELPFILEPPGeneral;
	m_dwHelpContext = HELPCONTEXT_PPGeneral;
	m_dwDocStringID = IDS_DOCSTRINGPPGeneral;
}

void CPPGeneral::InitControls()
{
	CComDispatchDriver prvctl(m_ppUnk[0]);

	ATLTRACE(_T("CPPGeneral::InitControls\n"));
	if (prvctl.p)
	{
		VARIANT v;

		EnterCriticalSection(&cs);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_ALLOWUSERZOOM, &v)))
			PutDlgCheckBoxBool(m_hWnd, IDC_ALLOWUSERZOOM, v.bVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_ENABLED, &v)))
			PutDlgCheckBoxBool(m_hWnd, IDC_ENABLED, v.bVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_BORDERSTYLE, &v)))
			PutDlgComboEnum(m_hWnd, IDC_BORDERSTYLE, v.iVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_MOUSEPOINTER, &v)))
		{
			if (v.iVal == 99)	// adjust value for "Custom"
				v.iVal = 16;
			PutDlgComboEnum(m_hWnd, IDC_MOUSEPOINTER, v.iVal);
		}

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_SCROLLBARH, &v)))
			PutDlgComboEnum(m_hWnd, IDC_SCROLLBARH, v.iVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_SCROLLBARV, &v)))
			PutDlgComboEnum(m_hWnd, IDC_SCROLLBARV, v.iVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_SCROLLINCH, &v)))
			PutDlgEditLong(m_hWnd, IDC_SCROLLINCH, v.lVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_SCROLLINCV, &v)))
			PutDlgEditLong(m_hWnd, IDC_SCROLLINCV, v.lVal);

		LeaveCriticalSection(&cs);
	}
	SetDirty(FALSE);
}

void CPPGeneral::LoadComboStrings(void)
{
	HWND hWndBorderStyle = GetDlgItem(IDC_BORDERSTYLE);
	HWND hWndMousePointer = GetDlgItem(IDC_MOUSEPOINTER);
	HWND hWndScrollBarH = GetDlgItem(IDC_SCROLLBARH);
	HWND hWndScrollBarV = GetDlgItem(IDC_SCROLLBARV);

	ATLTRACE(_T("CPGeneral::LoadComboStrings\n"));
#ifdef SPREAD_JPN
	::SendMessage(hWndBorderStyle, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("なし"));
	::SendMessage(hWndBorderStyle, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("実線"));

	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("既定値"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("矢印"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｸﾛｽ"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("I ﾋﾞｰﾑ"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｱｲｺﾝ"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｻｲｽﾞ(上下左右)"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｻｲｽﾞ(右上-左下)"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｻｲｽﾞ(上下)"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｻｲｽﾞ(左上-右下)"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｻｲｽﾞ(左右)"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("上矢印"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("砂時計"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("禁止"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("矢印と砂時計"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("矢印と疑問符"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ｻｲｽﾞ(全方向)"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ﾕｰｻﾞ定義"));

	::SendMessage(hWndScrollBarH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("表示"));
	::SendMessage(hWndScrollBarH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("自動表示"));
	::SendMessage(hWndScrollBarH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("非表示"));

	::SendMessage(hWndScrollBarV, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("表示"));
	::SendMessage(hWndScrollBarV, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("自動表示"));
	::SendMessage(hWndScrollBarV, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("非表示"));
#else
	::SendMessage(hWndBorderStyle, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("None"));
	::SendMessage(hWndBorderStyle, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Fixed Single"));

	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Default"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Arrow"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Cross"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("IBeam"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Icon"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Sizing"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("SizeNESW"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("SizeNS"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("SizeNWSE"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("SizeWE"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("UpArrow"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Hourglass"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("NoDrop"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ArrowHourglass"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ArrowQuestion"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("SizeAll"));
	::SendMessage(hWndMousePointer, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Custom"));

	::SendMessage(hWndScrollBarH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Show"));
	::SendMessage(hWndScrollBarH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Auto"));
	::SendMessage(hWndScrollBarH, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Hide"));

	::SendMessage(hWndScrollBarV, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Show"));
	::SendMessage(hWndScrollBarV, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Auto"));
	::SendMessage(hWndScrollBarV, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Hide"));
#endif
}

HRESULT CPPGeneral::Apply(void)
{
	ATLTRACE(_T("CPPGeneral::Apply\n"));
	for (UINT i = 0; i < m_nObjects; i++)
	{
		CComDispatchDriver prvctl(m_ppUnk[i]);

		if (prvctl.p)
		{
			VARIANT v;

			EnterCriticalSection(&cs);

			v.vt = VT_I2;
			v.iVal = GetDlgCheckBoxBool(m_hWnd, IDC_ALLOWUSERZOOM);
			prvctl.PutProperty(PRV_DISPID_ALLOWUSERZOOM, &v);

			v.iVal = GetDlgCheckBoxBool(m_hWnd, IDC_ENABLED);
			prvctl.PutProperty(PRV_DISPID_ENABLED, &v);

			v.iVal = GetDlgComboEnum(m_hWnd, IDC_BORDERSTYLE);
			prvctl.PutProperty(PRV_DISPID_BORDERSTYLE, &v);

			v.iVal = GetDlgComboEnum(m_hWnd, IDC_MOUSEPOINTER);
			if (v.iVal == 16)	// adjust value for "Custom"
				v.iVal = 99;
			prvctl.PutProperty(PRV_DISPID_MOUSEPOINTER, &v);

			v.iVal = GetDlgComboEnum(m_hWnd, IDC_SCROLLBARH);
			prvctl.PutProperty(PRV_DISPID_SCROLLBARH, &v);

			v.iVal = GetDlgComboEnum(m_hWnd, IDC_SCROLLBARV);
			prvctl.PutProperty(PRV_DISPID_SCROLLBARV, &v);

			v.vt = VT_I4;
			v.lVal = GetDlgEditLong(m_hWnd, IDC_SCROLLINCH);
			prvctl.PutProperty(PRV_DISPID_SCROLLINCH, &v);

			v.lVal = GetDlgEditLong(m_hWnd, IDC_SCROLLINCV);
			prvctl.PutProperty(PRV_DISPID_SCROLLINCV, &v);

			LeaveCriticalSection(&cs);
		}
	}
	ReInitControls();
	return S_OK;
}
