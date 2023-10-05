#pragma once


#include "DialogExt.h"
#include "afxwin.h"
/////////////////////////////////////////////////////////////////////////////
// CTabDlgSpecActTesting dialog
class CPipes;
class CDS_HydroMod;
class CDS_HydroMod::CBV;
class CDS_HydroMod::CCv;
class CDS_HydroMod::CDpC;

class CTabDlgSpecActTesting : public CDialogExt
{
public:
	enum { IDD = IDD_TABSPECACT_TESTING };

	CTabDlgSpecActTesting( CWnd *pParent = NULL );

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedBrowse();
	virtual afx_msg void OnBnClickedStart();
	virtual afx_msg void OnBnClickedStop();
	virtual afx_msg void OnBnClickedHMSnapshoot();
	virtual afx_msg void OnBnClickedBatchSnapshoot();
	virtual afx_msg void OnSize( UINT nType, int cx, int cy );

// Private methods.
private:
	void _ProcessBatch( CString strfn );
	void _Parse();
	bool _InterpretCmd(CString *pstrReturn );

	void _DropHMContent( CString strfn, bool bClearFile = false);	
	void _FillHM( CFileTxt *pfw, CTable *pTab = NULL );

	void _WriteHMInfo(CFileTxt *pfw, CDS_HydroMod *pHM );
	void _WritePipeInfo(CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation );
	void _WritePipeAccessoryInfo(CFileTxt *pfw, CString strLineHeader, CDS_HydroMod *pHM, CDS_HydroMod::eHMObj ePipeLocation );
	void _WriteDpCInfo(CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CDpC *pHMDpC );
	void _WriteCVInfo(CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CCv *pHMCV );
	void _WriteBVInfo(CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CBV *pHMBV );
	void _WriteShutOffValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CShutoffValve *pHMShutoffValve );
	void _WriteSmartControlValveInfo( CFileTxt *pfw, CString strLineHeader, CDS_HydroMod::CSmartControlValve *pHMSmartControlValve );

	bool _ProcessHydroMod( CString *pRetStr );
	bool _ProcessCheckValue( CString *pRetStr, double dHMValue );
	
// Private variables.
private:
	CEdit m_EditFileName;
	CListBox m_List;
	std::map<int, CString> m_mapStr;
	CStringArray m_arStr;
	CStringArray m_arBatch;
	CString m_RootFolder;

	bool m_bRun;
};
