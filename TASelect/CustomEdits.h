#pragma once


// CNumEdit

class CNumEdit : public CEdit
{
	DECLARE_DYNAMIC(CNumEdit)

public:
	CNumEdit();
	virtual ~CNumEdit();

protected:
	double m_dMin,m_dMax,m_dVal;
	bool CheckValidity(CString &str, double &dVal);

	DECLARE_MESSAGE_MAP()
public:
	void SetMinMax(double dMin, double dMax); 
	double GetValue();

	afx_msg void OnEnChange();
};


