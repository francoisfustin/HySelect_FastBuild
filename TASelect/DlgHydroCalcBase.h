#pragma once

// Base class for 'CDlgHydroCalcQKvDp', 'CDlgHydroCalcQPDT' and 'CDlgHydroCalcRedDisk'.
class CDlgHydroCalc;
class CDlgHydroCalcBase : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgHydroCalcBase )

public:
	class CMyRadioButton : public CButton
	{
		// Remark: We don't use 'CButton' class for radio buttons in the three hydronic calculation dialogs.
		//         Usually when we need radio button, we create a group and put radio buttons in it. First button must have 'Group' and 'Tab stop'
		//         properties set to 'TRUE' and these properties must be set to 'FALSE' for others.
		//         If we let MFC to automatically manage these buttons, when playing with TAB key, focus can bet set on the active radio button
		//         (just at the right of the radio button but just before text area even if text is empty!).
		//         Event if we create three separate buttons and we disable 'tab stop' property for them, it is yet possible to have focus when clicking
		//         on the button. For this, we just use this class to override 'OnSetFocus' to do nothing with it.
		DECLARE_MESSAGE_MAP()
		afx_msg void OnSetFocus( CWnd *pOldWnd );
	};

	class CMyEdit : public CEdit
	{
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	};

	CDlgHydroCalcBase( UINT nIDTemplate, CDlgHydroCalc *pDlgHydroCalc );
	void Calculate( void );

	virtual void OnChangeActiveTab( void ) = 0;

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	// Do here because some are commons in the three dialogs.
	afx_msg void OnEnChangeEditFlow();
	afx_msg void OnEnChangeEditKv();
	afx_msg void OnEnChangeEditDp();
	afx_msg void OnEnChangeEditPower();
	afx_msg void OnEnChangeEditDT();
	virtual void OnOK();
	virtual void OnCancel();

	void CalculateQKvDp( void );
	void CalculateQPowerDT( void );
	virtual void CalculateRedDisk( void ) {}
	bool ValidateInput( ReadDoubleReturn_enum eReturn, double& dValue, CEdit& clEditField );

protected:
	CDlgHydroCalc *m_pDlgHydroCalc;
	CMyRadioButton m_Radio1;
	CMyRadioButton m_Radio2;
	CMyRadioButton m_Radio3;
	CMyEdit m_EditFlow;
	CMyEdit m_EditKv;
	CMyEdit m_EditDp;
	CMyEdit m_EditPower;
	CMyEdit m_EditDT;
	bool m_bBlockEnHandlers;
};
