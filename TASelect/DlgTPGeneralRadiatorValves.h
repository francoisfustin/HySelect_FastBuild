#pragma once


#include "DlgCtrlPropPage.h"
#include "ExtNumEdit.h"

class CDlgTPGeneralRadiatorValves : public CDlgCtrlPropPage
{
	DECLARE_DYNAMIC( CDlgTPGeneralRadiatorValves )

public:
	enum { IDD = IDD_TABDIALOGTECHRAD };

	CDlgTPGeneralRadiatorValves( CWnd* pParent = NULL );

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
	CTADatastruct*	m_pTADS;

	CExtNumEdit		m_EditAvailableDp;
	CExtNumEdit		m_EditRoomTemp;
	CExtNumEdit		m_EditSupplyTemp;

	double			m_dTrvDefDpTot;
	double			m_dTrvDefRoomT;
	double			m_dTrvDefSupplyT;
};
