#pragma once

#include "ExtComboBox.h"
#include "ExtNumEdit.h"
#include "afxcmn.h"
#include "afxbutton.h"

class CDlgTALink : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgTALink )

public:
	class CMyEdit : public CEdit
	{
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	};

	class CMyExtEdit : public CExtNumEdit
	{
	public:
		CMyExtEdit() : CExtNumEdit() { m_bEnabled = true; }
		void EnableMouseEvent( bool fEnabled ) { m_bEnabled = fEnabled; }

	protected:
		DECLARE_MESSAGE_MAP()
		afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );
		afx_msg void OnRButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnRButtonDblClk( UINT nFlags, CPoint point );
		afx_msg void OnMButtonDown( UINT nFlags, CPoint point );
		afx_msg void OnMButtonDblClk( UINT nFlags, CPoint point );
		afx_msg BOOL OnSetCursor( CWnd *pWnd, UINT nHitTest, UINT message );

	private:
		bool m_bEnabled;
	};

	enum { IDD = IDD_DLGTALINK };

	CDlgTALink( CWnd *pParent = NULL );

	virtual ~CDlgTALink();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedOk();
	BOOL PreTranslateMessage( MSG* pMsg );
	afx_msg void OnEnChangeEditSetting();
	afx_msg void OnEnChangeEditFlow();
	afx_msg void OnEnChangeEditOutput();
	afx_msg void OnSelChangeComboValveType();
	afx_msg void OnSelChangeComboValveFamily();
	afx_msg void OnSelChangeComboValve();
	afx_msg void OnSelChangeComboTALink();
	afx_msg void OnClickedMFCChoiceButton();

private:
	void _InitializeTALinkList( void );
	void _FillComboValveType( CString strValveTypeID = _T("") );
	void _FillComboValveFamily( CString strValveFamilyID = _T("") );
	void _FillComboValve( CString strValveID = _T("") );
	void _FillComboTALink( void );
	void _Calculate( int iComeFrom );
	void _CalculateKv( void );
	void _CalculateOutput( void );
	void _CalculateFlow( void );

private:
	
	enum ComeFrom
	{
		EditValve,
		EditSetting,
		EditTALink,
		EditFlow,
		EditOutput
	};

	enum TLUnit
	{
		Volt		= 0x0000,
		A			= 0x0001,
	};

	enum LastInput
	{
		Flow,
		OutputSignal,
		NotYet
	};

	struct TALinkProduct
	{
		TLUnit eUnit;
		double dLowDpValue;
		double dHighDpValue;
		double dLowOutputValue;
		double dHighOutputValue;
		CString strName;
	};

	typedef std::vector<TALinkProduct*>	vecTLProduct;

	vecTLProduct m_vecTALinkList;
	bool m_bBlockEnHandlers;
	CDB_TAProduct *m_pValve;
	CExtNumEditComboBox m_ComboValveType;
	CExtNumEditComboBox m_ComboValveFamily;
	CExtNumEditComboBox m_ComboValve;
	CComboBox m_ComboTALink;
	CMyEdit m_EditSetting;
	CEdit m_EditKv;
	CMyExtEdit m_EditFlow;
	CMyExtEdit m_EditOutput;
	CMyExtEdit m_EditDp;
	CEdit m_EditDpMax;
	double m_dSetting;
	double m_dFlow;
	double m_dOutput;
	double m_dKvComputed;
	double m_dDpComputed;
	bool m_bOutputError;
	LastInput m_eLastInput;
	TALinkProduct *m_pCurrentTALinkProduct;
	CWaterChar *m_pclWaterChar;
	CMFCButton m_MFCChoiceButton;
	CButton m_ButtonOK;
};
