#pragma once


// Class CMFCRibbonBarEx create to interpret the right click button
class CMFCRibbonBarEx : public CMFCRibbonBar
{
public:
	// Intercept the Right click button.
	virtual BOOL OnShowRibbonContextMenu( CWnd *pWnd, int x, int y, CMFCRibbonBaseElement *pHit );
	virtual BOOL PreTranslateMessage( MSG *pMsg );
};
