#pragma once
#include <vector>

enum AdditionalDataForControl
{
	AdData_CMFCTabCtrl,
	AdData_CTreeCtrl,
	AdData_CMFCRibbonBar,
	AdData_Last
};

class CMacroBaseClassData
{
public:
	CMacroBaseClassData() {}
	virtual ~CMacroBaseClassData() {}
	virtual bool Write( CFile* pclFile );
	virtual bool Read( CFile* pclFile );
};

class CMacroMFCTabCtrlData : public CMacroBaseClassData
{
public:
	CMacroMFCTabCtrlData() {}
	virtual ~CMacroMFCTabCtrlData() {}
	virtual bool Write( CFile* pclFile );
	virtual bool Read( CFile* pclFile );
	int m_iTabID;
};

class CMacroTreeCtrlData : public CMacroBaseClassData
{
public:
	CMacroTreeCtrlData() {}
	virtual ~CMacroTreeCtrlData() {}
	virtual bool Write( CFile* pclFile );
	virtual bool Read( CFile* pclFile );
	DWORD_PTR m_dwParam;
};

class CMacroMFCRibbonBarData : public CMacroBaseClassData
{
public:
	CMacroMFCRibbonBarData() {}
	virtual ~CMacroMFCRibbonBarData() {}
	virtual bool Write( CFile* pclFile );
	virtual bool Read( CFile* pclFile );
};

class CMacroWndInfo
{
public:
	CMacroWndInfo();
	virtual ~CMacroWndInfo();

	bool Write( CFile* pclFile );
	bool Read( CFile* pclFile );

	int m_iID;
	CString m_strClassName;
	int m_iAdDataID;
	CMacroBaseClassData *m_pclAdData;
};

typedef std::vector<CMacroWndInfo*> vecWndInfo;
typedef vecWndInfo::iterator vecWndInfoIter;

class CMacro
{
public:
	CMacro() {}
	virtual ~CMacro() {}

	bool Write( CFile* pclFile );
	bool Read( CFile* pclFile );

	MSG m_msg;
	CPoint m_pt;
	DWORD m_dwTickCount;
	vecWndInfo m_vecWndHierarchy;
};
	
typedef std::vector<CMacro*> vecMacro;
typedef vecMacro::iterator vecMacroIter;
