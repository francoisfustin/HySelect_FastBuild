
#pragma once

// Class CMFCRibbonPanelEx create to add the possibility to change dynamically the text
class CMFCRibbonPanelEx : public CMFCRibbonPanel
{
	//DECLARE_DYNAMIC(CMFCRibbonPanelEx)
	//CMFCRibbonPanelEx();
public:
	
	void SetText(LPCTSTR lpszText);
	LPCTSTR GetText() const { return m_strName; }
};