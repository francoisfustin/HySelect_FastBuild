#pragma once


class CDlgInjectionError : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgInjectionError )

public:
	enum { IDD = IDD_DLGINJECTIONERROR };

	enum class ReturnCode
	{
		Undefined,
		ApplyWithoutCorrection,
		ApplyWithCorrection,
		Cancel
	};

	CDlgInjectionError( std::vector<CDS_HydroMod *> *pvecAllInjectionCircuitsWithTemperatureError, bool bDisplayListErrors = true, CWnd *pParent = nullptr );
	virtual ~CDlgInjectionError() {}
	ReturnCode GetReturnCode() { return m_eReturnCode; }

protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedApplyWithoutCorrection();
	afx_msg void OnBnClickedApplyWithCorrection();
	afx_msg void OnBnClickedCancel();

// Protected variables.
protected:
	std::vector<CDS_HydroMod *> *m_pvecAllInjectionCircuitsWithTemperatureError;
	bool m_bDisplayListErrors;
	ReturnCode m_eReturnCode;

	CListBox m_clListBoxErrors;
	CButton m_ButtonApplyWithoutCorrection;
	CButton m_ButtonApplyWithCorrection;
	CButton m_ButtonCancel;

// Private methods.
private:
	void _FillErrorList();
};
