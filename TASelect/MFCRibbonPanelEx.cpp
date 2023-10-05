// MFCRibbonPanelEx.cpp : implementation file
//

#include "stdafx.h"
#include "TASelect.h"

#include "MFCRibbonPanelEx.h"

// CMFCRibbonPanelEx
void CMFCRibbonPanelEx::SetText(LPCTSTR lpszText)
{
	ASSERT_VALID(this);
	m_strName = lpszText == NULL ? L"" : lpszText;
}