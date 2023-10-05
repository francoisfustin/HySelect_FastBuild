#pragma once


#include "ExtStatic.h"

class CDlgExceptionMessage : public CDialogEx
{
	DECLARE_DYNAMIC( CDlgExceptionMessage )

public:
	enum { IDD = IDD_DLGEXCEPTIONMESSAGE };

	CDlgExceptionMessage( bool bCurrentProjectSaved, bool bMiniDumpCreated, CString strTimestamp, CWnd *pParent = NULL );
	virtual ~CDlgExceptionMessage();

// Protected methods.
protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

// Private variables.
private:
	bool m_bCurrentProjectSaved;
	bool m_bMiniDumpCreated;
	CString m_strTimestamp;
	HICON m_hProgramIcon;
	CExtStatic m_StaticProgramIcon;
	CExtStatic m_ExtStaticMainTitleBackground;
	CExtStatic m_ExtStaticMainTitleText;
	CExtStatic m_ExtStaticSubTitleBackground;
	CExtStatic m_ExtStaticSubTitleText;
	CExtStatic m_ExtStaticMsgProjectSaveStatus;
	CExtStatic m_ExtStaticMsgProjectSaveFile;
	CExtStatic m_ExtStaticMsgCrashDumpStatus;
	CExtStatic m_ExtStaticMsgCrashDumpFile;
	CExtStatic m_ExtStaticMsgCrashDumpSendReport;
};
