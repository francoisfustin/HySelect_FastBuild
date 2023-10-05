#pragma once


#include "EnBitmap.h"

class CDlgHydronicSchViewLarge : public CDialogEx
{
public:
	enum { IDD = IDD_DLGHYDRONICSCHVIEWLARGE };

	CDlgHydronicSchViewLarge( CEnBitmap *pBitmap, CWnd *pParent = NULL );
	virtual ~CDlgHydronicSchViewLarge() {}

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonRefresh();

// Private variables.
private:
	CEnBitmap *m_pBitmap;
	CEnBitmap m_clBitmap;
	CStatic m_stDynSch;
};
