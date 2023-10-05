// PPPage.cpp : Implementation of CPPPage
#include "stdafx.h"
#include "PPPage.h"

/////////////////////////////////////////////////////////////////////////////
// CPPPage
CPPPage::CPPPage() 
{
	m_dwTitleID = IDS_TITLEPPPage;
	m_dwHelpFileID = IDS_HELPFILEPPPage;
	m_dwHelpContext = HELPCONTEXT_PPPage;
	m_dwDocStringID = IDS_DOCSTRINGPPPage;
}

void CPPPage::InitControls()
{
	CComDispatchDriver prvctl(m_ppUnk[0]);
	BOOL b;

	ATLTRACE(_T("CPPPage::InitControls\n"));
	if (prvctl.p)
	{
		VARIANT v;

		EnterCriticalSection(&cs);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEVIEWTYPE, &v)))
			PutDlgComboEnum(m_hWnd, IDC_PAGEVIEWTYPE, v.iVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEVIEWPERCENTAGE, &v)))
			PutDlgEditInt(m_hWnd, IDC_PAGEVIEWPERCENTAGE, v.iVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEBORDERWIDTH, &v)))
			PutDlgEditInt(m_hWnd, IDC_PAGEBORDERWIDTH, v.iVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGESHADOWWIDTH, &v)))
			PutDlgEditInt(m_hWnd, IDC_PAGESHADOWWIDTH, v.iVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEGUTTERH, &v)))
			PutDlgEditLong(m_hWnd, IDC_PAGEGUTTERH, v.lVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEGUTTERV, &v)))
			PutDlgEditLong(m_hWnd, IDC_PAGEGUTTERV, v.lVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEMULTICNTH, &v)))
			PutDlgEditInt(m_hWnd, IDC_PAGEMULTICNTH, v.iVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_PAGEMULTICNTV, &v)))
			PutDlgEditInt(m_hWnd, IDC_PAGEMULTICNTV, v.iVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_GRAYAREAMARGINTYPE, &v)))
			PutDlgComboEnum(m_hWnd, IDC_GRAYAREAMARGINTYPE, v.iVal);
		
		if (!FAILED(prvctl.GetProperty(PRV_DISPID_GRAYAREAMARGINH, &v)))
			PutDlgEditLong(m_hWnd, IDC_GRAYAREAMARGINH, v.lVal);

		if (!FAILED(prvctl.GetProperty(PRV_DISPID_GRAYAREAMARGINV, &v)))
			PutDlgEditLong(m_hWnd, IDC_GRAYAREAMARGINV, v.lVal);
		
		LeaveCriticalSection(&cs);
	}
	OnPageViewTypeChange(0,0,0,b);
	SetDirty(FALSE);
}

void CPPPage::LoadComboStrings(void)
{
	HWND hWndPageViewType = GetDlgItem(IDC_PAGEVIEWTYPE);
	HWND hWndGrayAreaMarginType = GetDlgItem(IDC_GRAYAREAMARGINTYPE);

	ATLTRACE(_T("CPPPage::LoadComboStrings\n"));
#ifdef SPREAD_JPN
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Õﬂ∞ºﬁëSëÃ"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("∫›ƒ€∞ŸÇÃª≤Ωﬁ"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T(" ﬂ∞æ›√∞ºﬁ"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Õﬂ∞ºﬁÇÃïù"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Õﬂ∞ºﬁÇÃçÇÇ≥"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("ï°êîÕﬂ∞ºﬁ"));

	::SendMessage(hWndGrayAreaMarginType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Ωπ∞ÿ›∏ﬁ"));
	::SendMessage(hWndGrayAreaMarginType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("é¿ç€ÇÃª≤Ωﬁ"));
#else
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Whole Page"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Normal Size"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Percentage"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Page Width"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Page Height"));
	::SendMessage(hWndPageViewType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Multiple Pages"));

	::SendMessage(hWndGrayAreaMarginType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Scaled"));
	::SendMessage(hWndGrayAreaMarginType, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)_T("Actual"));
#endif
}

LRESULT CPPPage::OnPageViewTypeChange(WORD wNotify, WORD wID, HWND hWnd, BOOL& bHandled)
{
	short n = GetDlgComboEnum(m_hWnd, IDC_PAGEVIEWTYPE);
	
	ATLTRACE(_T("CPPPage::OnPageViewTypeChange\n"));
	if (2 == n)
	{	// page view type percentage
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEVIEWPERCENTAGE), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEVIEWPERCENTAGE), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERS), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEGUTTERH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERV), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEGUTTERV), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGESINVIEW), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEMULTICNTH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEMULTICNTH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEMULTICNTV), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEMULTICNTV), FALSE);
	}
	else if (5 == n)
	{	// page view type multiple pages
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEVIEWPERCENTAGE), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEVIEWPERCENTAGE), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERS), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERH), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEGUTTERH), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERV), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEGUTTERV), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGESINVIEW), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEMULTICNTH), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEMULTICNTH), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEMULTICNTV), TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEMULTICNTV), TRUE);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEVIEWPERCENTAGE), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEVIEWPERCENTAGE), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERS), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEGUTTERH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEGUTTERV), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEGUTTERV), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGESINVIEW), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEMULTICNTH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEMULTICNTH), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_STATIC_PAGEMULTICNTV), FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd, IDC_PAGEMULTICNTV), FALSE);
	}
	// delegate handling to EnableApply()
	return EnableApply(wNotify, wID, hWnd, bHandled);
}

HRESULT CPPPage::Apply(void)
{
	ATLTRACE(_T("CPPPage::Apply\n"));
	for (UINT i = 0; i < m_nObjects; i++)
	{
		CComDispatchDriver prvctl(m_ppUnk[i]);

		if (prvctl.p)
		{
			VARIANT v;

			EnterCriticalSection(&cs);

			v.vt = VT_I2;
			v.iVal = GetDlgComboEnum(m_hWnd, IDC_PAGEVIEWTYPE);
			prvctl.PutProperty(PRV_DISPID_PAGEVIEWTYPE, &v);

			v.iVal = GetDlgEditInt(m_hWnd, IDC_PAGEVIEWPERCENTAGE);
			prvctl.PutProperty(PRV_DISPID_PAGEVIEWPERCENTAGE, &v);

			v.iVal = GetDlgEditInt(m_hWnd, IDC_PAGEBORDERWIDTH);
			prvctl.PutProperty(PRV_DISPID_PAGEBORDERWIDTH, &v);

			v.iVal = GetDlgEditInt(m_hWnd, IDC_PAGESHADOWWIDTH);
			prvctl.PutProperty(PRV_DISPID_PAGESHADOWWIDTH, &v);

			v.vt = VT_I4;
			v.lVal = GetDlgEditLong(m_hWnd, IDC_PAGEGUTTERH);
			prvctl.PutProperty(PRV_DISPID_PAGEGUTTERH, &v);
			
			v.lVal = GetDlgEditLong(m_hWnd, IDC_PAGEGUTTERV);
			prvctl.PutProperty(PRV_DISPID_PAGEGUTTERV, &v);

			v.vt = VT_I2;
			v.iVal = GetDlgEditInt(m_hWnd, IDC_PAGEMULTICNTH);
			prvctl.PutProperty(PRV_DISPID_PAGEMULTICNTH, &v);

			v.iVal = GetDlgEditInt(m_hWnd, IDC_PAGEMULTICNTV);
			prvctl.PutProperty(PRV_DISPID_PAGEMULTICNTV, &v);

			v.iVal = GetDlgComboEnum(m_hWnd, IDC_GRAYAREAMARGINTYPE);
			prvctl.PutProperty(PRV_DISPID_GRAYAREAMARGINTYPE, &v);

			v.vt = VT_I4;
			v.lVal = GetDlgEditLong(m_hWnd, IDC_GRAYAREAMARGINH);
			prvctl.PutProperty(PRV_DISPID_GRAYAREAMARGINH, &v);
			
			v.lVal = GetDlgEditLong(m_hWnd, IDC_GRAYAREAMARGINV);
			prvctl.PutProperty(PRV_DISPID_GRAYAREAMARGINV, &v);

			LeaveCriticalSection(&cs);
		}
	}
	ReInitControls();
	return S_OK;
}

