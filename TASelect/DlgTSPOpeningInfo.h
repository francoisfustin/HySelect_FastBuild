#pragma once


#include "DlgListBox.h"
#include "DeletedProductHelper.h"

class CDlgTSPOpeningInfo : public CDlgListBox, CDeletedProductHelper
{
public:
	CDlgTSPOpeningInfo( CWnd* pParent = NULL );
	virtual ~CDlgTSPOpeningInfo();

	void Display( bool bForUnitTest );
	
	// HYS-1291 : Lock all hydromod deleted product
	void LockDeletedProductForHydromod();
// Private members.
private:
	void _FillListCtrl();
	virtual afx_msg void OnOK();
};

