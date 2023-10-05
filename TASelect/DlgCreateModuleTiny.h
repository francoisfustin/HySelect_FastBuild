#pragma once


#include "CbiRestrEdit.h"
#include "ExtComboBox.h"

class CDlgCreateModuleTiny : public CDialogEx
{
public:
	enum { IDD = IDD_DLGCREATEMODULETINY };

	enum DialogMode
	{
		Undefined,
		CreateModule,
		AddValve,
	};

	CDlgCreateModuleTiny( CWnd* pParent = NULL );	 // Standard constructor.

	//	Display :   DlgMode			to select between creation, edition, valve and module.
	//				strCaption		dialog title.
	//				SelTADSThingID	current selected valve.
	int Display( DialogMode eDlgMode, double dFlow, CString strSelTADSThingID, enum_VDescriptionType eVDescrType, double dKvCv, double dPresetting = -1.0 );

	double GetFlow() { return m_dFlow; }
	double GetKvCv() { return m_dKvCv; }
	double GetPresetting() { return m_dSelPresetting; }
	CString GetSelTADBID() { return m_strSelTADBID; }
	void SetSelTADBID( CString ID ) { m_strSelTADBID = ID; }
	bool IsValveChanged() { return m_fValveChanged; }
	enum_VDescriptionType GetVDescrType() { return m_eVDescrType; }

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnMove( int x, int y );
	afx_msg void OnCheckPartnerValve();
	afx_msg void OnEnChangeFlow();
	afx_msg void OnButtonKvCvInput();
	afx_msg void OnButtonDirectSelection();

// Private methods.
private:
	bool _CheckEditQ();

// Private variables.
private:
	CButton			m_CheckPartnerValve;
	CEdit			m_EditFlow;
	CEdit			m_EditValve;
	CButton			m_ButtonKvCvInput;
	CButton			m_ButtonDirectSel;

	CTADatabase*	m_pTADB;
	CTADatastruct*	m_pTADS;
	CUnitDatabase*	m_pUnitDB;
	DialogMode		m_eDlgMode;
	bool			m_bInitialized;
	double			m_dFlow;
	double			m_dSelPresetting;
	double			m_dKvCv;
	bool			m_fValveChanged;
	CString			m_strSelTADBID;
	CString			m_strSelTADBCategory;
	CString			m_strCBIType;
	CString			m_strCBISize;
	enum_VDescriptionType m_eVDescrType;
};
