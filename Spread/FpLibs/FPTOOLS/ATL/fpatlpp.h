/****************************************************************************
* FPATLPP.H  -  Declaration of FarPoint's CfpAtlPropPage template class
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
//  2.  06/23/99	SCL -	Changed to derive from CAxDialogImpl instead of CDialogImpl

#ifndef __FPATLPROPPAGE_H_
#define __FPATLPROPPAGE_H_

//	You must link with fptools for ATL. -scl
#include "atlppbas.h"	// for CfpAtlPropPageBase

// fix for bug 9717 -scl
#ifdef HTMLHELP
#include <htmlhelp.h>
extern CComModule _Module; // defined in main CPP
#endif

template <class T, const CLSID* pclsid> // as on CComCoClass
class ATL_NO_VTABLE CfpAtlPropPage :
	public CfpAtlPropPageBase,
	public CComObjectRoot,
	public CComCoClass<T, pclsid>,
	public IPropertyPageImpl<T>,
	public CAxDialogImpl<T>
{
private:
	// pointer to parent class
	T*		m_pT;
protected:
	// flag to prevent enabling Apply button when
	// controls are being initialized with values.
	// prevents enabling the Apply button when TRUE.
	BOOL	m_fInitializing;
   BOOL  m_fInSetObjects;
public:
	CfpAtlPropPage():
	  m_pT(static_cast<T*>(this))
	  {
		  m_fInitializing = TRUE;
        m_fInSetObjects = FALSE;
	  };

//	You must declare the following 
//	in your derived class
//	to associate a dialog:
//	enum {IDD = <your dialog ID here> };

//	You must declare the following
//	in your derived class
//	for your prop page to get registered:
//DECLARE_REGISTRY_RESOURCEID(IDR_BLNAPPEARANCEPROPPAGE)

BEGIN_COM_MAP(T) 
	COM_INTERFACE_ENTRY_IMPL(IPropertyPage)
END_COM_MAP()

//	You will need a message map in your derived class too.
//	You must map the appropriate notification 
//	(ie. EN_CHANGE, CBN_SELCHANGE, etc.)
//	for all the controls in your dialog to EnableApply() (see below).
//	In your derived class:
//BEGIN_MSG_MAP(<Derived class name>)
//	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//  MESSAGE_HANDLER(WM_HELP, OnWmHelp)
//  <Your message handlers, all mapped to EnableApply>
//	CHAIN_MSG_MAP(IPropertyPageImpl<Derived class name>)
//END_MSG_MAP()

protected:
//	Override of the flawed ATL implementation.
//	void SetDirty(BOOL bDirty)
//	{
//		if (m_bDirty ^ bDirty)
//		{
//			m_bDirty = bDirty;
//			if (m_pPageSite)
//			{
//				if( bDirty )
//					m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
//				else
//					m_pPageSite->OnStatusChange(PROPPAGESTATUS_CLEAN);
//			}
//		}
//	}

//	Message handler for all CHANGE notifications from dialog controls.
	LRESULT EnableApply(WORD wNotify, WORD wID, HWND hWnd, BOOL& bHandled)
	{
		if (!m_fInitializing)
			SetDirty(TRUE);
		return 0;
	}

//	Required function override: put code here 
//	to load strings into the ComboBox controls
//	int your property page dialog
	virtual void LoadComboStrings(void) = 0;
/* Sample implementation:
void CfpSamplePropPage::LoadComboStrings(void)
{
	ATLTRACE(_T("CfpSamplePropPage::LoadComboStrings\n"));
	HWND	hWndMyEnumProp = GetDlgItem(IDC_COMBO_MY_ENUM_PROP);
	// repeat for additional comboboxes

	::SendMessage(hWndMyEnumProp, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("String 1"));
	// . . . //
	::SendMessage(hWndMyEnumProp, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("String n"));
	// repeat for additional comboboxes
}
*/

//	Required function override: put code here 
//	to initialize controls in the dialog with 
//	the appropriate values.
//	IMPORTANT: Do not forget to enter and leave
//	the critical section in CfpAtlPropPageBase.
	virtual void InitControls() = 0;
/* Sample implementation:
void CfpSamplePropPage::InitControls(void)
{
	ATLTRACE(_T("CfpSamplePropPage::InitControls\n"));
	CComQIPtr<IfpSample, &IID_IfpSample> pSample(m_ppUnk[0]);
	if (pSample)
	{
		// temporary variables for getting property values
		short			nVal;
		float			flVal;
		int				iVal;
		long			lVal;
		VARIANT_BOOL	bVal;
		BSTR			bstr;

		// Enter the critical section in CfpAtlPropPageBase
		EnterCriticalSection(&cs);

		// the helper functions referenced here are defined below.
		// an enumerated property using a ComboBox control:
		pSample->get_MyEnumProp((enumMyEnumProp*)&nVal);
		PutDlgComboEnum(m_hWnd, IDC_COMBO_MY_ENUM_PROP, nVal);
		
		// a float property using an Edit control:
		pSample->get_MyFloatProp(&flVal);
		PutDlgEditFloat(m_hWnd, IDC_EDIT_MY_FLOAT_PROP, flVal);

		// an int property using an Edit control:
		pSample->get_MyIntProp(&iVal);
		PutDlgIntVal(m_hWnd, IDC_EDIT_MY_INT_PROP, iVal);

		// a BSTR prop using an Edit control:
		pSample->get_MyBSTRProp(&bstr);
		PutDlgEditBstr(m_hWnd, IDC_EDIT_MY_BSTR_PROP, bstr);

		// a VARIANT_BOOL prop using a CheckBox control:
		pSample->put_MyBoolProp(GetDlgCheckBoxBool(m_hWnd, IDC_CHECK_MY_BOOL_PROP));
		
		// Leave the critical section in CfpAtlPropPageBase
		LeaveCriticalSection(&cs);
	}
	SetDirty(FALSE);
	return S_OK;
}
*/

//	You may need to override to initialize ComboBox controls
//	in your dialog with strings.  For some reason ATL prop pages
//	do not load this data even if you setup the lists in the 
//	dialog editor.
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pT->LoadComboStrings();
		m_fInitializing = TRUE;
		m_pT->InitControls();
		m_fInitializing = FALSE;
		bHandled = TRUE;
		return TRUE;
	}

//	If property values on a given page can change due to
//	changes made to other pages, then these values will
//	need to be refreshed when the given page is activated
//	again.  To do this, map the WM_SETFOCUS message for the
//	first control on the page to this function, which will
//	call InitControls().
	LRESULT ReInitControls(WORD wNotify, WORD wID, HWND hWnd, BOOL& bHandled)
	{
		if (m_bDirty)
		{
			m_fInitializing = TRUE;
			m_pT->InitControls();
			m_fInitializing = FALSE;
		}
		bHandled = TRUE;
		return 0;
	}

//	Overloaded with no params or return val, so that you can call 
//	it through code without having to declare local variables
	void ReInitControls(void)
	{
		if (m_bDirty) 
		{
			BOOL b;
			ReInitControls(0,0,0,b);
		}
	}

	//STDMETHOD(Show)(UINT nCmdShow)
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		//if (nCmdShow != SW_HIDE)
		ATLTRACE(_T("CfpPropPageBase::OnShowWindow\n"));
		if (wParam)
			ReInitControls();
		//return IPropertyPageImpl<T>::Show(nCmdShow);
		bHandled = TRUE;
		return 0;
	}

//	Required function override: put code here to set the appropriate
//	properties on the controls.
//	IMPORTANT: Do not forget to enter and leave
//	the critical section in CfpAtlPropPageBase.
	STDMETHOD(Apply)(void);

/*	Sample implementation:
HRESULT CfpSamplePropPage::Apply(void)
{
	ATLTRACE(_T("CfpSamplePropPage::Apply\n"));
	for (UINT i = 0; i < m_nObjects; i++)
	{
		CComQIPtr<IfpSample, &IID_IfpSample> pSample(m_ppUnk[i]);
		if (pSample)
		{
			// Enter critical section in CfpAtlPropPageBase
			EnterCriticalSection(&cs);

			// temporary variables for getting dialog data
			// and performing type conversions.
			BSTR bstr;	// you must free BSTRs return by GetDlgEditBstr()

			// an enumerated property using a ComboBox control:
			pSample->put_MyEnumProp((enumMyEnumProp)GetDlgComboEnum(m_hWnd, IDC_COMBO_MY_ENUM_PROP));
			
			// a float property using an Edit control:
			pSample->put_MyFloatProp(GetDlgEditFloat(m_hWnd, IDC_EDIT_MY_FLOAT_PROP));

			// an int property using an Edit control:
			pSample->put_MyIntProp(GetDlgEditInt(m_hWnd, IDC_EDIT_MY_INT_PROP));

			// an long property using an Edit control:
			pSample->put_MyLongProp(GetDlgEditLong(m_hWnd, IDC_EDIT_MY_INT_PROP));

			// a BSTR prop using an Edit control:
			pSample->put_MyBSTRProp(bstr = GetDlgEditBstr(m_hWnd, IDC_EDIT_MY_BSTR_PROP));
			SysFreeString(bstr);

			// a VARIANT_BOOL prop using a CheckBox control:
			pSample->put_MyBoolProp(GetDlgCheckBoxBool(m_hWnd, IDC_CHECK_MY_BOOL_PROP));
			
			// Leave the critical section in CfpAtlPropPageBase
			LeaveCriticalSection(&cs);
			SetDirty(FALSE);
		}
	}
	InitControls();
	return S_OK;
}
*/

// fix for bug 9717 -scl
#ifdef HTMLHELP
  private:
  void GetHelpFilePath(LPTSTR szPath)
  {
    BSTR bstr = NULL;
    CComPtr<ITypeLib> pTypeLib;
    
    if( SUCCEEDED(AtlModuleLoadTypeLib(&_Module, NULL, &bstr, &pTypeLib)) )
    {
      if( bstr )
        SysFreeString(bstr);
      pTypeLib->GetDocumentation(-1, NULL, NULL, NULL, &bstr);
      if( bstr )
      {
#ifdef _UNICODE
        lstrcpy(szPath, bstr);
#else
        WideCharToMultiByte(CP_ACP, 0, bstr, SysStringLen(bstr), szPath, _MAX_PATH, NULL, NULL);
#endif
        SysFreeString(bstr);
      }
    }
  }
  protected:
#endif

// implements context help for the controls in the dialogs.
// the help context is the control ID of the active control
	STDMETHOD(Help)(LPCOLESTR pszHelpDir)
	{
		DWORD dwContext = ::GetDlgCtrlID(::GetFocus());
		TCHAR  szHelpFile[_MAX_PATH];

		ATLTRACE(_T("CfpPropPageBase::Help\n"));
// fix for bug 9717 -scl
#ifdef HTMLHELP
    GetHelpFilePath(szHelpFile);
		if (dwContext > 10000 || dwContext == 9)
			// go to general help context
      HtmlHelp(m_pT->m_hWnd, szHelpFile, HH_HELP_CONTEXT, m_pT->m_dwHelpContext);
		else
      HtmlHelp(m_pT->m_hWnd, szHelpFile, HH_HELP_CONTEXT, dwContext);
#else
		if (!LoadString(_Module.GetResourceInstance(), m_pT->m_dwHelpFileID, szHelpFile, _MAX_PATH))
		{
			ATLTRACE(_T("Error : Failed to load help filename string from res\n"));
			return E_FAIL;
		}
		if (dwContext > 10000 || dwContext == 9)
			// go to general help context
			WinHelp(szHelpFile, HELP_CONTEXT, m_pT->m_dwHelpContext);
		else
			WinHelp(szHelpFile, HELP_CONTEXT, dwContext);
#endif
		return S_OK;
	}

	STDMETHOD(SetObjects)(ULONG nObjects, IUnknown** ppUnk)
	{
		HRESULT hr = S_OK;

// this was causing recursion in the BtnObjx property pages,
// so I commented it out -scl
//GAB 8/9/04 Uncommented to fix bug 14241
//      SetDirty(TRUE);
		m_bDirty = TRUE;
		if (S_OK == (hr = IPropertyPageImpl<T>::SetObjects(nObjects, ppUnk)))
		{
			ReInitControls();
		}
		SetDirty(FALSE);
		return hr;
	}

// handle the WM_HELP message sent when F1 is pressed
	LRESULT OnWmHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPHELPINFO lphi = (LPHELPINFO)lParam;
		char  szHelpFile[_MAX_PATH];

		ATLTRACE(_T("CfpPropPageBase::OnWmHelp\n"));
// fix for bug 9717 -scl
#if HTMLHELP
    GetHelpFilePath(szHelpFile);
		if (!lphi->dwContextId || lphi->dwContextId > 10000 || lphi->dwContextId == 9)
			// go to general help context
      HtmlHelp(m_pT->m_hWnd, szHelpFile, HH_HELP_CONTEXT, m_pT->m_dwHelpContext);
		else
			// got to help context for this property
      HtmlHelp(m_pT->m_hWnd, szHelpFile, HH_HELP_CONTEXT, lphi->dwContextId);
#else
		if (!LoadString(_Module.GetResourceInstance(), m_pT->m_dwHelpFileID, szHelpFile, _MAX_PATH))
		{
			ATLTRACE(_T("Error : Failed to load help filename string from res\n"));
			return 0;
		}
		if (!lphi->dwContextId || lphi->dwContextId > 10000 || lphi->dwContextId == 9)
			// go to general help context
			WinHelp(szHelpFile, HELP_CONTEXT, m_pT->m_dwHelpContext);
		else
			// got to help context for this property
			WinHelp(szHelpFile, HELP_CONTEXT, lphi->dwContextId);
#endif
		return 0;
	}
};

#endif //__FPATLPROPPAGE_H_