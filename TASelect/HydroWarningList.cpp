
#include "stdafx.h"
#include "Utilities.h"
#include "TASelect.h"
#include "DataBase.h"

#include "HydroWarningList.h"

CHydroWarningList::CHydroWarningList(void)
{
	m_array.SetSize(100,100);
}

CHydroWarningList::~CHydroWarningList(void)
{
}

void CHydroWarningList::ClearAll()
{
	m_array.RemoveAll();
}
bool CHydroWarningList::DeleteAt(int pos)
{
	ASSERT (pos>=m_array.GetCount());
	if (pos>=m_array.GetCount()) return false;
	m_array.RemoveAt(pos);
	return true;
}

int	CHydroWarningList::GetCount()
{
	return (m_array.GetCount());
}
CString CHydroWarningList::GetMsgAt(int pos)
{
	sWarningMsg wMsg;
	ASSERT (pos>=m_array.GetCount());
	if (pos>=m_array.GetCount()) return _T("");
	wMsg = m_array.GetAt(pos);
	return wMsg.msg;
};
IDPTR CHydroWarningList::GetIDPtrAt(int pos)
{
	sWarningMsg wMsg;
	ASSERT (pos>=m_array.GetCount());
	if (pos>=m_array.GetCount()) return _NULL_IDPTR;
	wMsg = m_array.GetAt(pos);
	return wMsg.idptr;
};
UINT CHydroWarningList::GetSeverityAt(int pos)
{
	sWarningMsg wMsg;
	ASSERT (pos>=m_array.GetCount());
	if (pos>=m_array.GetCount()) return 0;
	wMsg = m_array.GetAt(pos);
	return wMsg.svrty;
};
bool CHydroWarningList::GetAt(int pos,eHydroWarning *phw,CString *pStr, IDPTR *pIDPtr, UINT *pSeverity/*=NULL*/)
{
	sWarningMsg wMsg;
	ASSERT (pos>=m_array.GetCount());
	if (pos>=m_array.GetCount()) return false;
	wMsg = m_array.GetAt(pos);
	if (phw) *phw = wMsg.hw;
	if (pStr) *pStr = wMsg.msg;
	if (pIDPtr) *pIDPtr = wMsg.idptr;
	if (pSeverity) *pSeverity = wMsg.svrty;
	return true;
};
bool CHydroWarningList::GetAt(int pos,sWarningMsg *pwMsg)
{
	sWarningMsg wMsg;
	ASSERT (pos>=m_array.GetCount());
	if (pos>=m_array.GetCount()) return false;
	if (!pwMsg) return false;
	wMsg = m_array.GetAt(pos);
	*pwMsg = wMsg;
	return true;
}

int	CHydroWarningList::Add(eHydroWarning hw,CString msg, IDPTR idptr, UINT Severity/*=0*/)
{
	int pos=-1;
	// Verify that new obj not already exist in the current array
	bool find = false;
	for (int i=0; i<m_array.GetCount() && !find; i++)
	{
		sWarningMsg &wMsg = m_array.ElementAt(i);
		if (wMsg.hw == hw && !_tcscmp(wMsg.idptr.ID,idptr.ID)) 
			find = true;
	}
	if (!find)
	{
		sWarningMsg wMsg,wExistingMsg;
		wMsg.hw = hw;
		wMsg.idptr = idptr;
		wMsg.msg = msg;
		wMsg.svrty = Severity;
		pos = m_array.Add(wMsg);
	}
	return pos;
};
int	CHydroWarningList::Add(eHydroWarning hw,int IDmsg, IDPTR idptr, UINT Severity/*=0*/)
{
	CString str; str=TASApp.LoadLocalizedString(IDmsg);
	return Add(hw, str, idptr, Severity);
};


