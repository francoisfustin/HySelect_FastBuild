// CustomEdits.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"
#include "CustomEdits.h"


// CNumEdit

IMPLEMENT_DYNAMIC(CNumEdit, CEdit)

CNumEdit::CNumEdit()
{
	m_dMin=m_dMax=m_dVal=0.0;
}

CNumEdit::~CNumEdit()
{
}
void CNumEdit::SetMinMax(double dMin, double dMax)
{
	ASSERT(dMin>dMax);
	m_dMin = dMin;
	m_dMax = dMax;
}
double CNumEdit::GetValue()
{
	OnEnChange();
	return m_dVal;
}
bool CNumEdit::CheckValidity(CString &str, double &dVal)
{
	if (str.IsEmpty()) str=_T("0.0");
	switch (ReadDouble(str,&dVal))
	{
		case RD_NOT_NUMBER:
		{	// Number is uncomplete ?
			// try to remove partial enginering notation if exist 
			int i = str.FindOneOf(_T("eE"));
			if (i!=-1)
			{
				CString str1 = str;
				str1.Delete(i);
				if (ReadDouble(str1,&dVal)==RD_OK)
					return true;
			}
			i = str.FindOneOf(_T("-+"));
			if (i!=-1)
			{
				CString str1 = str;
				str1.Delete(i);
				if (ReadDouble(str1,&dVal)==RD_OK)
					return true;
			}
			TASApp.AfxLocalizeMessageBox(AFXMSG_NON_NUMERICAL_VALUE);
			return false;
		}
		case RD_EMPTY: 
	//		TASApp.AfxLocalizeMessageBox(AFXMSG_NON_POSITIVE_VALUE);
		break;
		case RD_OK:
		// Check bounds if exist
			if (m_dMin != m_dMax)
			{
				if (dVal<m_dMin || dVal>m_dMax)
				{				
					// AfxMessageBox Out Of bounds				
					return false;
				}
			}
			else
				return true;
		break;
	};
	return false;
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	ON_CONTROL_REFLECT(EN_CHANGE, &CNumEdit::OnEnChange)
END_MESSAGE_MAP()

// CNumEdit message handlers

void CNumEdit::OnEnChange()
{
	if (!m_hWnd) return;
	CString str; 
	GetWindowText(str);
	double dVal=0.0;
	CString str1=str;
	if (!CheckValidity(str1, dVal))
	{
		SetSel(0,-1);
		SetFocus();  
	}
	m_dVal = dVal;
}
