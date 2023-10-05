/****************************************************************************
* ATLPPBAS.CPP  -  Implmentation of FarPoint's CfpAtlPropPageBase class
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
//	1.	03/06/98	SCL	-	Created
//
#include "stdafx.h"
#include "atlppbas.h"
#include "stdlib.h"
#include "fptools.h"
#include "fptstr.h"

CRITICAL_SECTION	CfpAtlPropPageBase::cs		= { 0 };
int					CfpAtlPropPageBase::nRefs	= 0;
/////////////////////////////////////////////////////////////////////////////
// CfpAtlPropPageBase - Base class of static functions for use with ATL
//						property page dialogs
BOOL CfpAtlPropPageBase::PutDlgComboEnum(HWND hWndDlg, int nIDDlgCtrl, short nVal)
{
	if (nVal == GetDlgComboEnum(hWndDlg, nIDDlgCtrl))
		return TRUE;
	else
	{
		BOOL bRet;
		int bSelItem = ::SendMessage(::GetDlgItem(hWndDlg, nIDDlgCtrl), CB_SETCURSEL, (WPARAM)nVal, 0L); 
		bRet = (bSelItem == nVal);
		return bRet;
	}
}

short CfpAtlPropPageBase::GetDlgComboEnum(HWND hWndDlg, int nIDDlgCtrl)
{
	short sRet = (short)::SendMessage(::GetDlgItem(hWndDlg, nIDDlgCtrl), CB_GETCURSEL, 0, 0L);
	return sRet;
}

BOOL CfpAtlPropPageBase::PutDlgEditInt(HWND hWndDlg, int nIDDlgCtrl, int iVal, BOOL fSigned)
{
//	if (iVal == GetDlgEditInt(hWndDlg, nIDDlgCtrl, NULL, fSigned))
//		return TRUE;
//	else
	{
		BOOL bRet = (BOOL)::SetDlgItemInt(hWndDlg, nIDDlgCtrl, (UINT)iVal, fSigned);
		return bRet;
	}
}

int CfpAtlPropPageBase::GetDlgEditInt(HWND hWndDlg, int nIDDlgCtrl, BOOL* lpTrans, BOOL fSigned)
{
	int nRet = ::GetDlgItemInt(hWndDlg, nIDDlgCtrl, lpTrans, fSigned);
	return nRet;
}

BOOL CfpAtlPropPageBase::PutDlgEditLong(HWND hWndDlg, int nIDDlgCtrl, long lVal)
{
//	if (lVal == GetDlgEditLong(hWndDlg, nIDDlgCtrl))
//		return TRUE;
//	else
	{
		TCHAR buf[sizeof(TCHAR) * 34];
		BOOL bRet;
#ifdef UNICODE
		_ltow(lVal, buf, 10);
#else
		_ltoa(lVal, buf, 10);
#endif
		bRet = (BOOL)::SetDlgItemText(hWndDlg, nIDDlgCtrl, buf);
		return bRet;
	}
}

long CfpAtlPropPageBase::GetDlgEditLong(HWND hWndDlg, int nIDDlgCtrl)
{
	TCHAR buf[sizeof(TCHAR) * 34];
	long lRet;

	::GetDlgItemText(hWndDlg, nIDDlgCtrl, buf, 33);
	lRet = atol(buf);
	return lRet;
}

BOOL CfpAtlPropPageBase::PutDlgEditFloat(HWND hWndDlg, int nIDDlgCtrl, float flVal)
{
//	if (flVal == GetDlgEditFloat(hWndDlg, nIDDlgCtrl))
//		return TRUE;
//	else
	{
		VARIANT v;
		FPTSTR  fpStr;
		BOOL	fRet;

		v.vt = VT_R4;
		v.fltVal = flVal;
		if (S_OK == VariantChangeType((VARIANTARG FAR*)&v, (VARIANTARG FAR*)&v, 0, VT_BSTR))
		{
			fpStr = fpSysAllocTStringFromBSTR((LPVOID)v.bstrVal);
			fRet = ::SetDlgItemText(hWndDlg, nIDDlgCtrl, (LPCTSTR)fpStr);
			SysFreeString(v.bstrVal);
			fpSysFreeTString(fpStr);
		}
		return fRet;
	}
}

float CfpAtlPropPageBase::GetDlgEditFloat(HWND hWndDlg, int nIDDlgCtrl)
{
	TCHAR	buf[255];
	VARIANT	v;
	float	flRet = 0;

	if (::GetDlgItemText(hWndDlg, nIDDlgCtrl, (LPTSTR)buf, 255))
	{
		v.vt = VT_BSTR;
		v.bstrVal = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)buf);
		if (S_OK == VariantChangeType((VARIANTARG FAR*)&v, (VARIANTARG FAR*)&v, 0, VT_R4))
			flRet = v.fltVal;
		// freed by VariantChangeType() -scl
		//SysFreeString(v.bstrVal);
	}
	return flRet;
}

BOOL CfpAtlPropPageBase::PutDlgEditBstr(HWND hWndDlg, int nIDDlgCtrl, BSTR bstr, BOOL fAutoRelease)
{
	BOOL fRet;
	TCHAR buf[255];

	FPTSTR fpStr = fpSysAllocTStringFromBSTR((LPVOID)bstr);

	::GetDlgItemText(hWndDlg, nIDDlgCtrl, buf, 255);
	if (!lstrcmp(buf, fpStr))
		fRet = TRUE;
	else
		fRet = ::SetDlgItemText(hWndDlg, nIDDlgCtrl, (LPCTSTR)fpStr);
	fpSysFreeTString(fpStr);
	if (fAutoRelease)
		SysFreeString(bstr);
	return fRet;
}

BSTR CfpAtlPropPageBase::GetDlgEditBstr(HWND hWndDlg, int nIDDlgCtrl)
{
	TCHAR buf[255];
	BSTR bstrRet;

	::GetDlgItemText(hWndDlg, nIDDlgCtrl, buf, 255);
	bstrRet = (BSTR)fpSysAllocBSTRFromTString((FPTSTR)buf);
	return bstrRet;
}

BOOL CfpAtlPropPageBase::PutDlgCheckBoxBool(HWND hWndDlg, int nIDDlgCtrl, BOOL bVal)
{
	if (bVal == GetDlgCheckBoxBool(hWndDlg, nIDDlgCtrl))
		return TRUE;
	else
	{
		BOOL bRet = ::CheckDlgButton(hWndDlg, nIDDlgCtrl, bVal ? BST_CHECKED : BST_UNCHECKED);
		return bRet;
	}
}

VARIANT_BOOL CfpAtlPropPageBase::GetDlgCheckBoxBool(HWND hWndDlg, int nIDDlgCtrl)
{
	VARIANT_BOOL vbRet = (BST_CHECKED == ::IsDlgButtonChecked(hWndDlg, nIDDlgCtrl));
	return vbRet;
}

