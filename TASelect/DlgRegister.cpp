//
// DlgRegister.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TASelect.h"
#include "utilities.h"
#include "BlowFish.h"
#include "RegAccess.h"
#include "DlgRegister.h"

#ifdef DEBUG
#include "DlgSpecAct.h"
#include "TabDlgSpecActDev.h"
#include "TabDlgSpecActUser.h"
#endif

// CDlgRegister dialog

IMPLEMENT_DYNAMIC(CDlgRegister, CDialogEx)
CDlgRegister::CDlgRegister(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRegister::IDD, pParent)
{
	memset(m_BFKey,0,sizeof(m_BFKey));
	m_bMachineDependant = true;
}

CDlgRegister::~CDlgRegister()
{
}

void CDlgRegister::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITAC1, m_EditAC1);
	DDX_Control(pDX, IDC_EDITAC2, m_EditAC2);
	DDX_Control(pDX, IDC_EDITAC3, m_EditAC3);
	DDX_Control(pDX, IDC_EDITAC4, m_EditAC4);
	DDX_Control(pDX, IDC_EDITDC, m_EditDC);
}


BEGIN_MESSAGE_MAP(CDlgRegister, CDialogEx)
	ON_EN_CHANGE(IDC_EDITAC1, OnEnChangeEditac1)
	ON_EN_CHANGE(IDC_EDITAC2, OnEnChangeEditac2)
	ON_EN_CHANGE(IDC_EDITAC3, OnEnChangeEditac3)
	ON_EN_CHANGE(IDC_EDITAC4, OnEnChangeEditac4)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
//	ON_WM_SIZE()
END_MESSAGE_MAP()

// Create AC (Activation code)
void CDlgRegister::SetAC(USHORT nYear /*=1*/, BYTE nMonth /*=3*/)
{
	// AC Encoding B0..B7
	// B0..B4 DC(B0..B4)
	// B5	 Number of validity month
	// B6,B7 Key Validity Date ->	b15..b9 year	(7 bits 0..99 start at 2000)
	//								b8..b5	month	(4 bits 1..12)
	//								b4..b0  day		(5 bits 1..31)
	_uDate dt;
	memset(m_encryptedAC,0,sizeof(m_encryptedAC));
	memset(m_AC,0,sizeof(m_AC));

	// Copy DC into AC
	memcpy(m_AC,&m_DC[0],5);
	m_AC[5] = nMonth;
	CTime t = CTime::GetCurrentTime();
	dt.bf.nDay = t.GetDay();
	dt.bf.nMonth = t.GetMonth();
	dt.bf.nYear = t.GetYear()-2000 + nYear;
	// Copy into AC
	memcpy(&m_AC[6],&dt.usDate,sizeof(dt.usDate));

	memcpy(m_encryptedAC,m_AC,sizeof(m_encryptedAC));

	CBlowFish bf;
	bf.init(m_BFKey,sizeof(m_BFKey));
	int ret = bf.sencrypt(m_encryptedAC,sizeof(m_encryptedAC),"cbc");
}

// Create DC (Device code)
void CDlgRegister::SetDC()
{
	// DC Encoding 8 bytes B7..B0
	// B0..B2 = Language key without -
	// B3..B7 = 5 random number
	CString langstr=TASApp.GetTADBKey();
	memset(m_encryptedDC,0,sizeof(m_encryptedDC));
	memset(m_DC,0,sizeof(m_DC));
	// If code is undependant of machine it's dependant of localized version (language key)
	if (!m_bMachineDependant)
	{
		int j=0;
		for (int i=0; i<langstr.GetLength()&&j<3; i++)
		{
			if(langstr.GetAt(i) == CString(_T("-"))) continue;
			m_DC[j] = (UCHAR)langstr.GetAt(i);
			j++;
		}
	}
	else
	{
		m_DC[0] = 'N';
		m_DC[1] = 'E';
		m_DC[2] = 'U';
	}
	//Seed the random-number generator with current time so that
	//the numbers will be different every time we run.
	srand( (unsigned)time( NULL ) );
	for (int i=3; i<8; i++)
		if (m_bMachineDependant)
			m_DC[i] = (unsigned char)rand();
		else
			m_DC[i] = i;

	memcpy(m_encryptedDC,m_DC,sizeof(m_encryptedDC));
	CBlowFish bf;
	bf.init(m_BFKey,sizeof(m_BFKey));
	int ret = bf.sencrypt(m_encryptedDC,sizeof(m_encryptedDC),"cbc");
}

void CDlgRegister::CreateRegKeyIfNeeded()
{
	// Create keys if needed
	BYTE data[8];
	DWORD dwDispo=0;
	HKEY RegKey;
	LONG lret;
	// Common Activation
	CString Keypath = _T("Software\\IMI Hydronic Engineering\\HySelect Common");
	lret = ::RegCreateKeyEx(HKEY_CURRENT_USER,(LPCTSTR)Keypath,0,NULL,REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS,NULL,&RegKey,&dwDispo);
	ASSERT (lret == (LONG) ERROR_SUCCESS);
	if (RegKey) :: RegCloseKey(RegKey);
	if (dwDispo == REG_CREATED_NEW_KEY)
	{
		memset (data,0,sizeof(data));
		CRegAccess::SetRegBinData(Keypath,_T("Device Code"),data, sizeof(data),false);
		CRegAccess::SetRegBinData(Keypath,_T("Activation Code"),data, sizeof(data),false);
	}
	// Local Activation
	Keypath = _T("Software\\IMI Hydronic Engineering\\");
	Keypath += TASApp.GetTADBKey()+_T("\\HySelect Common");
	lret = ::RegCreateKeyEx(HKEY_CURRENT_USER,(LPCTSTR)Keypath,0,NULL,REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS,NULL,&RegKey,&dwDispo);
	ASSERT (lret == (LONG) ERROR_SUCCESS);
	if (RegKey) :: RegCloseKey(RegKey);
	if (dwDispo == REG_CREATED_NEW_KEY)
	{
		memset (data,0,sizeof(data));
		CRegAccess::SetRegBinData(Keypath,_T("Device Code"),data, sizeof(data),false);
		CRegAccess::SetRegBinData(Keypath,_T("Activation Code"),data, sizeof(data),false);
	}
}

void CDlgRegister::UpdateRegistry()
{
	CRegAccess::SetRegBinData(m_Regstr,_T("Device Code"),m_encryptedDC, sizeof(m_encryptedDC),false);
	CRegAccess::SetRegBinData(m_Regstr,_T("Activation Code"),m_encryptedAC, sizeof(m_encryptedAC),false);
}

// Initialise variable and test if system is unlocked
long CDlgRegister::Init()
{
	int ret = -1;
	m_pTADB = TASApp.GetpTADB();			ASSERT(m_pTADB);
	CString str;
	CDB_MultiString* pHMCalcAct = (CDB_MultiString *) m_pTADB->Get(_T("HMCALCACTIVAT")).MP;
	str = pHMCalcAct->GetString(0);
	for(int i=0; i<str.GetLength(); i++)
		m_BFKey[i] = (unsigned char)str.GetAt(i);
	//Is Machine dependant
	str = pHMCalcAct->GetString(1);
	str.MakeLower();

	if (str == _T("true"))
		m_bMachineDependant=true;
	else
		m_bMachineDependant=false;

	memset(m_encryptedDC,0,sizeof(m_encryptedDC));
	memset(m_encryptedAC,0,sizeof(m_encryptedAC));
	memset(m_DC,0,sizeof(m_DC));
	memset(m_AC,0,sizeof(m_AC));

	CreateRegKeyIfNeeded();
	// Information is stored in TASelect common key 
	UINT n;
	bool bret=false;

	//if (m_bMachineDependant)
	{
		m_Regstr = _T("Software\\IMI Hydronic Engineering\\HySelect Common");
		// Retrieve DC (device code) and AC (Activation code)
		bret = CRegAccess::GetRegBinData(m_Regstr,_T("Device Code"),m_encryptedDC,&n,false);
		bret &= CRegAccess::GetRegBinData(m_Regstr,_T("Activation Code"),m_encryptedAC,&n,false);
		// Verify code is different of 0
		if (bret)
		{
			bret=false;		
			for (int i=0;i<sizeof(m_encryptedAC)&&!bret;i++)
				if (m_encryptedAC[i]) 
				{	
					bret=true;
				}
		}
	}
	// If code is undependant of machine, it's dependant of localized version (language key)
	if (!m_bMachineDependant && !bret)
	{
		m_Regstr = _T("Software\\IMI Hydronic Engineering\\");
		m_Regstr += TASApp.GetTADBKey()+_T("\\HySelect Common");

		bret = CRegAccess::GetRegBinData(m_Regstr,_T("Device Code"),m_encryptedDC,&n,false);
		bret &= CRegAccess::GetRegBinData(m_Regstr,_T("Activation Code"),m_encryptedAC,&n,false);
		// Verify code is different of 0
		if (bret)
		{
			bret=false;		
			for (int i=0;i<sizeof(m_encryptedAC)&&!bret;i++)
				if (m_encryptedAC[i]) bret=true;
		}
	}

	if (bret)
	{	// Key exist
		// Decode
		CBlowFish bf,bf1;
		bf.init(m_BFKey,sizeof(m_BFKey));
		memcpy(m_DC,m_encryptedDC,sizeof(m_DC));
		memcpy(m_AC,m_encryptedAC,sizeof(m_AC));
		int ret = bf.sdecrypt(m_AC,sizeof(m_AC),"cbc");
		ret = bf.sdecrypt(m_DC,sizeof(m_DC),"cbc");
		ret = -1;
		// Verify AC versus DC test only 6 first byte
		for (int i=0; i<(sizeof(m_AC)-2); i++)
			if (m_AC[i] != m_DC[i])
				return ret;
		// Verify Date
		_uDate dt;
		dt.usDate = (m_AC[7]<<8)+m_AC[6];
		// Check date validity
		if (!dt.bf.nDay || dt.bf.nDay>31 || !dt.bf.nMonth || dt.bf.nMonth>12 || dt.bf.nYear<6 || dt.bf.nYear>99)
		{
			memset(m_AC,0,sizeof(m_AC));
			memset(m_encryptedAC,0,sizeof(m_encryptedAC));
			memset(m_DC,0,sizeof(m_DC));
			memset(m_encryptedDC,0,sizeof(m_encryptedDC));
			UpdateRegistry();
			return -1;
		}
		COleDateTime tdate = COleDateTime::GetCurrentTime();
		COleDateTime tregdate;
		tregdate.SetDate(dt.bf.nYear+2000,dt.bf.nMonth,dt.bf.nDay);
		COleDateTimeSpan ts= tregdate-tdate;
		if (ts.GetDays()>0) return ts.GetDays();
		else return 0;
	}
	return -1;
}

bool CDlgRegister::VerifyInputChar(CEdit *pe)
{
	CString str;
	CString HexAcsiiStr = CString(_T("0123456789ABCDEF"));
	pe->GetWindowText(str);
	// Verify each char
	bool flag = true;
	for (int i=0; i<str.GetLength(); i++)
	{
		TCHAR ch = str.GetAt(i);
		if (HexAcsiiStr.FindOneOf(&ch)==-1) 
		{
			flag = false;
			str.Delete(i);
		}
	}
	if (!flag)
	{	
		pe->SetWindowText(str);
		pe->SetSel(0,-1);
		pe->SetFocus();
		MessageBeep(MB_ICONEXCLAMATION);
		return false;
	}
	if (str.GetLength()==4)
		return true;
	return false;
}

bool CDlgRegister::VerifyUserAC()
{
	// Disable OK Button
	GetDlgItem(IDOK)->EnableWindow(false);
	// Verify field completion
	CString str,strAC;
	m_EditAC1.GetWindowText(str);
	strAC+=str;
	m_EditAC2.GetWindowText(str);
	strAC+=str;
	m_EditAC3.GetWindowText(str);
	strAC+=str;
	m_EditAC4.GetWindowText(str);
	strAC+=str;
	if (strAC.GetLength()!=(sizeof(m_encryptedUAC)*2))
		return false;
	// Read user activation code
	int pos = 0;
#pragma warning( disable : 4244)
	for (int i=0; i<strAC.GetLength(); i+=2)
	{
		BYTE b[2];
		b[0] = strAC.GetAt(i);
		b[1] = strAC.GetAt(i+1);
		m_encryptedUAC[pos]=HexAsciiToByte(b);
		pos++;
	};
#pragma warning( default : 4244)
	CBlowFish bf;
	bf.init(m_BFKey,sizeof(m_BFKey));
	int ret = bf.sdecrypt(m_encryptedUAC,sizeof(m_encryptedUAC),"cbc");
	// Verify user activation Key only 5 first bytes
	int i;
	for (i=0; i<5; i++)
	{
		if (m_DC[i]!=m_encryptedUAC[i])
			return false;
	}
	// Verify Key Activation date and create a new key that will be stored in the registry
	// Verify Date
	_uDate dt;
	dt.usDate = (m_encryptedUAC[7]<<8)+m_encryptedUAC[6];
	// Check date validity
	if (!dt.bf.nDay || dt.bf.nDay>31 || !dt.bf.nMonth || dt.bf.nMonth>12 || dt.bf.nYear<6 || dt.bf.nYear>99)
		return false;
	COleDateTime tdate = COleDateTime::GetCurrentTime();
	COleDateTime ValKeyDate;
	ValKeyDate.SetDate(dt.bf.nYear+2000,dt.bf.nMonth,dt.bf.nDay);
	COleDateTimeSpan ts= ValKeyDate-tdate;
	if (ts.GetDays()>0)//>ACTIVATIONWARNINGDAYS)
	{
		// Key is valid 
		// Compute expiration date
		ts.SetDateTimeSpan(m_encryptedUAC[5]*30,0,0,0);
		tdate+=ts;
		dt.bf.nYear = tdate.GetYear()-2000;
		dt.bf.nMonth = tdate.GetMonth();
		dt.bf.nDay = tdate.GetDay();
		int j;
		for (j=0; j<6; j++)
			m_AC[j] = m_DC[j];
		m_AC[6] = (BYTE)dt.usDate;
		m_AC[7] = (BYTE)(dt.usDate>>8);
		for (j=0; j<sizeof(m_encryptedAC); j++)
			m_encryptedAC[j] = m_AC[j];
		bf.sencrypt(m_encryptedAC,sizeof(m_encryptedAC),"cbc");
		// Enable OK Button
		GetDlgItem(IDOK)->EnableWindow(true);
		return true;
	}
	return false;
}

// CDlgRegister message handlers

void CDlgRegister::OnEnChangeEditac1()
{
	if (VerifyInputChar(&m_EditAC1))
	{
		m_EditAC2.SetFocus();
		VerifyUserAC();
	}
}

void CDlgRegister::OnEnChangeEditac2()
{
	if (VerifyInputChar(&m_EditAC2))
	{
		m_EditAC3.SetFocus();
		VerifyUserAC();
	}
}

void CDlgRegister::OnEnChangeEditac3()
{
	if (VerifyInputChar(&m_EditAC3))
	{
		m_EditAC4.SetFocus();
		VerifyUserAC();
	}
}

void CDlgRegister::OnEnChangeEditac4()
{
	if (VerifyInputChar(&m_EditAC4))
	{
		GetDlgItem(IDOK)->SetFocus();
		VerifyUserAC();
	}
}

BOOL CDlgRegister::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// Strings initialisation
	CString str;
	str=TASApp.LoadLocalizedString(IDS_DLGREGISTER_CAPTION);
	SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREGISTER_LABEL1);
	GetDlgItem(IDC_LABEL1)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREGISTER_LABEL2);
	GetDlgItem(IDC_LABEL2)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREGISTER_DEVCODE);
	GetDlgItem(IDC_STATICDC)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_DLGREGISTER_ACTCODE);
	GetDlgItem(IDC_STATICAC)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	m_EditAC1.SetWindowText(_T(""));
	m_EditAC2.SetWindowText(_T(""));
	m_EditAC3.SetWindowText(_T(""));
	m_EditAC4.SetWindowText(_T(""));

	m_EditAC1.SetLimitText(4);
	m_EditAC2.SetLimitText(4);
	m_EditAC3.SetLimitText(4);
	m_EditAC4.SetLimitText(4);

	m_EditDC.SetWindowText(_T("-"));

	// Disable OK Button
	GetDlgItem(IDOK)->EnableWindow(false);

	// Load medium TA logo bitmap and affect it 
	TASApp.GetCompanyLogo( &m_BitmapTaLogo, this );
	( (CStatic *)GetDlgItem( IDC_STATICTALOGO ) )->SetBitmap( (HBITMAP)m_BitmapTaLogo.GetSafeHandle() );
// 	m_BitmapTaLogo.LoadBitmap(IDB_LOGOTA_SCREEN);
// 	HBITMAP hBitmap = (HBITMAP) m_BitmapTaLogo.GetSafeHandle();
// 	((CStatic*)GetDlgItem(IDC_STATICTALOGO))->SetBitmap(hBitmap);

	// Reposition IDC_STATICTALOGO
	RECT ButtonRect,StaticRect;
	CButton *pButton = (CButton*)GetDlgItem(IDOK);
	pButton->GetWindowRect(&ButtonRect);
	ScreenToClient(&ButtonRect);
	CStatic *pStatic = (CStatic*)GetDlgItem(IDC_STATICTALOGO);
	pStatic->GetWindowRect(&StaticRect);
	ScreenToClient(&StaticRect);
	pStatic->SetWindowPos(&wndTop,StaticRect.left,ButtonRect.bottom-StaticRect.bottom+StaticRect.top,0,
		0,SWP_NOSIZE);

	GetDlgItem(IDC_AC)->ShowWindow(false);

#ifdef DEBUG
	CDlgSpecAct dlg;

	if (dlg.CheckFile())
	{
		GetDlgItem(IDC_AC)->ShowWindow(true);
	}
#endif

	// Variable initialization
	Init();
	// Create Device Code
	SetDC();
	char HexChar[2];
	if (!m_bMachineDependant)
	{	// Hide DC relative 
		m_EditDC.ShowWindow(false);
		GetDlgItem(IDC_STATICDC)->ShowWindow(false);
		GetDlgItem(IDC_LABEL2)->ShowWindow(false);
	}
	else
	{
		// Fill EditDC
		str.Empty();
		for (int i=0; i<sizeof(m_encryptedDC); i++)
		{
			ByteToHexAscii(m_encryptedDC[i],(BYTE *)HexChar);
			if (i!=0 &&(i%2)==0) str += CString(_T("-"));
			str += CString(HexChar[0]);
			str += CString(HexChar[1]);
		}
		m_EditDC.SetWindowText(str);
	}
	// For testing
	// Compute Activation Code
	SetAC();
	str.Empty();
	int j=0;
	for (int i=0; i<sizeof(m_encryptedAC); i++)
	{
		ByteToHexAscii(m_encryptedAC[i],(BYTE *)HexChar);
		if (j!=0&&(j%4)==0) 
			str += CString(_T("-"));
		str += CString(HexChar[0]);
		j++;
		if (j!=0&&(j%4)==0) 
			str += CString(_T("-"));
		str += CString(HexChar[1]);
		j++;
	}

#ifdef DEBUG
	if (dlg.CheckFile())
	{
		GetDlgItem(IDC_AC)->SetWindowText(str);
		CString str1 = str.Left(4);
		m_EditAC1.SetWindowText( str1 );
		str1 = str.Mid(5,4);
		m_EditAC2.SetWindowText( str1 );
		str1 = str.Mid(10,4);
		m_EditAC3.SetWindowText( str1 );
		str1 = str.Mid(15,4);
		m_EditAC4.SetWindowText( str1 );
	}
#endif	

	return TRUE;
}

void CDlgRegister::OnBnClickedOk()
{
	PREVENT_ENTER_KEY
	if (!VerifyUserAC())
	{
		GetDlgItem(IDOK)->EnableWindow(false);
		return;
	}
	UpdateRegistry();
	OnOK();
}


BOOL CDlgRegister::PreTranslateMessage(MSG* pMsg)
{
	// Process CTRL+V
	if(pMsg->message==WM_KEYDOWN ) // allows "system" keys like: ctrl, alt, tab...
	{	
		if(pMsg->wParam == 'V')
		{
			if(GetKeyState(VK_CONTROL) < 0)
			{
				// Test to see if we can open the clipboard first.
				if (OpenClipboard()) 
				{
					// Retrieve the Clipboard data (specifying that 
					// we want ANSI text (via the CF_TEXT value).
					HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);

					// Call GlobalLock so that to retrieve a pointer
					// to the data associated with the handle returned
					// from GetClipboardData.
					char *pchData = (char*)GlobalLock(hClipboardData);

					// Set a local CString variable to the data
					// and then update the dialog with the Clipboard data
					CString strFromClipboard = (TCHAR *)pchData;
					//m_edtFromClipboard.SetWindowText(strFromClipboard);

					// Unlock the global memory.
					GlobalUnlock(hClipboardData);

					// Finally, when finished I simply close the Clipboard
					// which has the effect of unlocking it so that other
					// applications can examine or modify its contents.
					CloseClipboard();

					// Analyze data should be AAAA-BBBB-CCCC-DDDD with only hexascii char
					strFromClipboard.MakeUpper();
					strFromClipboard.TrimLeft();
					strFromClipboard.TrimRight();

					if (strFromClipboard.GetLength() == 19 )
					{
						// Verify dash
						bool fOk = true;
						int pos = 4;
						for (int j=0; j<3 && true == fOk; j++)
						{
							if (strFromClipboard.GetAt(j+pos) != '-')
								fOk = false;
							pos+=4;
						}
						strFromClipboard.Remove('-');
						for (int i=0; i<strFromClipboard.GetLength() && true == fOk; i++ )
						{
							if (strFromClipboard.GetAt(i)<'0' || strFromClipboard.GetAt(i)>'F') 
							{
								fOk = false;
							}
						}

						if (true == fOk)
						{
							// String is valid Paste it
							CString str = strFromClipboard.Left(4);
							m_EditAC1.SetWindowText( str );
							str = strFromClipboard.Mid(4,4);
							m_EditAC2.SetWindowText( str );
							str = strFromClipboard.Mid(8,4);
							m_EditAC3.SetWindowText( str );
							str = strFromClipboard.Mid(12,4);
							m_EditAC4.SetWindowText( str );
							return TRUE;	
						}
					}
				}
			}
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
