#pragma once


class CDlgHydroCalcBase;
class CDlgHydroCalcQPDT : public CDlgHydroCalcBase
{
	DECLARE_DYNAMIC( CDlgHydroCalcQPDT )

public:

	enum { IDD = IDD_DLGHYDROCALCQPDT };

	enum RadioState
	{
		Flow = 0,
		Power,
		DT,
		Uninitialized = 0xFF
	};

	CDlgHydroCalcQPDT( CDlgHydroCalc *pDlgHydroCalc );

	// Overrides 'CDlgHydroCalcBase'.
	virtual void OnChangeActiveTab( void );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadios( UINT nID );
};
