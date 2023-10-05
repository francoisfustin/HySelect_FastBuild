
#pragma once

// Class CMFCVisualManagerOffice2007Ex create to set
// correctly a boolean when resource dll are loaded
class CMFCVisualManagerOffice2007Ex : public CMFCVisualManagerOffice2007
{
	//DECLARE_DYNCREATE(CMFCVisualManagerOffice2007Ex)
public:
	//CMFCVisualManagerOffice2007Ex();
	
	static void SetAutoFreeRes(BOOL bAutoFree = FALSE);
};