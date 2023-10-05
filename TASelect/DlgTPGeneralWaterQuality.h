#pragma once


#include "DlgCtrlPropPage.h"

class DlgTPGeneralWaterQuality : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( DlgTPGeneralWaterQuality )

public:
	enum { IDD = IDD_TABDIALOGTECHWQ };
	
	DlgTPGeneralWaterQuality( CWnd* pParent = NULL );

	// Overriding 'CDlgCtrlPropPage' methods.
	virtual void Init( bool fResetToDefault = false );
	virtual void Save( CString strSectionName );
	virtual bool VerifyInfo();

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

// Protected variables.
protected:
	CExtNumEdit m_clEditWaterHardness;
};
