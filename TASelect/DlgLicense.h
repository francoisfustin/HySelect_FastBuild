#pragma once


#include "ExtNumEdit.h"

class CDlgLicense : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgLicense )

public:
	enum { IDD = IDD_DLGLICENSE };
	CDlgLicense( CWnd *pParent = nullptr );
	virtual ~CDlgLicense() {}

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

protected:
	CExtNumEdit m_EditLicense;

private:
	void _ShowLicense( void );
};
