#pragma once

#include "ExtComboBox.h"

class CDlgHydroCalcBase;
class CDlgHydroCalcRedDisk : public CDlgHydroCalcBase
{
	DECLARE_DYNAMIC( CDlgHydroCalcRedDisk )

public:
	enum { IDD = IDD_DLGHYDROCALCREDDISK };

	enum RadioState
	{
		Flow = 0,
		Setting,
		Dp,
		Uninitialized = 0xFF
	};

	CDlgHydroCalcRedDisk( CDlgHydroCalc *pDlgHydroCalc );

	// Overrides 'CDlgHydroCalcBase'.
	virtual void OnChangeActiveTab( void );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadios( UINT nID );
	afx_msg void OnEnChangeEditSetting();
	afx_msg void OnSetFocusEditSetting();
	afx_msg void OnSelChangeComboValveType();
	afx_msg void OnSelChangeComboValveFamily();
	afx_msg void OnSelChangeComboValve();
	afx_msg void OnBnClickedDeletedValves();

	// Overrides 'CDlgHydroCalcBase'.
	virtual void CalculateRedDisk( void );

private:
	void _FillComboValveType( CString ValveTypeID = _T("") );
	void _FillComboValveFamily( CString ValveFamilyID = _T("") );
	void _FillComboValve( CString ValveID = _T("") );
	void _FindBestValve( void );

private:
	CMyEdit m_EditSetting;
	CExtNumEditComboBox m_ComboValveType;
	CExtNumEditComboBox m_ComboValveFamily;
	CExtNumEditComboBox m_ComboValve;
	double m_dPreviousKvs;
	int m_iPreviousRadio;

	// 'm_bSavePreviousRadioState' set to true if we must save radio button state. Typically, 'OnSelChangeComboValve' can explicitly be called
	// by 'OnSelChangeComboValveFamily' or by '_FindBestValve' methods.
	// Moving from a valve without Kv signal to a valve with Kv signal implies that we need to save previous radio button state. Because when
	// coming back to a valve without Kv signal we need to reset radio button state as it was before.
	// But, 'OnSelChangeComboValveFamily' calls '_FindBestValve' that itself will call 'OnSelChangeComboValve'. In this case, we MUST NOT save
	// previous radio button state. Because we have already changed current radio button in regards to this new context for valve with Kv signal.
	bool m_bSavePreviousRadioState;
	CButton m_CBDeletedValves;
};
