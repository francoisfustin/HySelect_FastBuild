#pragma once


#include "afxwin.h"
#include "xgroupbox.h"
#include "dlghydrocalcbase.h"


class CDlgHydroCalcBase;
class CDlgHydroCalcQKvDp : public CDlgHydroCalcBase
{
	DECLARE_DYNAMIC( CDlgHydroCalcQKvDp )

public:
	enum { IDD = IDD_DLGHYDROCALCQKVDP };

	enum RadioState
	{
		Flow = 0,
		Kv,
		Dp,
		Uninitialized = 0xFF
	};

	CDlgHydroCalcQKvDp( CDlgHydroCalc *pDlgHydroCalc );

	// Overrides 'CDlgHydroCalcBase'.
	virtual void OnChangeActiveTab( void );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadios( UINT nID );
};
