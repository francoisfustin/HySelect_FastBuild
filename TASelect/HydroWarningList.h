#pragma once

#include "afxtempl.h"

class CHydroWarningList
{
public:
	enum eHydroWarning
	{
		
		LastHydroWarning
	};
	struct sWarningMsg
	{
		eHydroWarning	hw;
		CString			msg;
		IDPTR			idptr;
		UINT			svrty;
	};

protected:
	CArray<sWarningMsg,sWarningMsg&> m_array;

public:
	CHydroWarningList(void);
	~CHydroWarningList(void);

	void	ClearAll();
	bool	DeleteAt(int pos);
	CString GetMsgAt(int pos);
	IDPTR	GetIDPtrAt(int pos);
	UINT	GetSeverityAt(int pos);
	int		GetCount();
	int		Add(eHydroWarning hw,CString msg, IDPTR idptr, UINT Severity=0);
	int		Add(eHydroWarning hw,int IDmsg, IDPTR idptr, UINT Severity=0);
	bool	GetAt(int pos,eHydroWarning *phw,CString *pStr, IDPTR *pIDPtr, UINT *pSeverity=NULL);
	bool	GetAt(int pos,sWarningMsg *pwMsg);
};
