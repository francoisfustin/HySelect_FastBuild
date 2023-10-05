#include "stdafx.h"
#include <windows.h>
#include <windowsx.h>
#include "TASelect.h"

#include "global.h"
#include "utilities.h"
#include "Hydromod.h"
#include "WizardCBI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CWizardCBI dialog
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CWizardCBI::CWizardCBI( CWnd* pParent )
	:CWizardManager( IDS_WIZCBI_TITLE, CWizardCBI::IDD )
{
	m_aTableUID.RemoveAll();
	m_CBIRxMsg.Reset();
	m_MsgList.RemoveAll();
	ZeroMemory( &m_HexAsciiTxMsg, 2 * BUFFERSIZE );
	m_pHATx = NULL;
	ZeroMemory( &m_HexAsciiRxMsg, 2 * BUFFERSIZE );
	m_pHARx = NULL;
	ZeroMemory( &m_PortAvailable, MAXCOMNUMBER * sizeof( bool ) );
	m_USflag = false;
	m_HPortCom = 0;
	ZeroMemory( &m_TxBuf, 200 * sizeof( BYTE) );
	ZeroMemory( &m_RxBuf, 200 * sizeof( BYTE) );

	Add( &m_PanelCBI1 );
	Add( &m_PanelCBI2 );
	Add( &m_PanelCBI3 );
	Add( &m_PanelCBIPlant );
	Add( &m_PanelCBILog );
}

void CWizardCBI::DoDataExchange(CDataExchange* pDX)
{
	CWizardManager::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWizardCBI, CWizardManager)
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER_INITCOMPORT, OnInitComPort)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWizardCBI member functions

bool CWizardCBI::IsCBIThere(s_Msg *pBuf)
{
	s_Msg tmp;
	memset(pBuf,0,sizeof(struct s_Msg));
	if(m_Com.GetComHandle()==INVALID_HANDLE_VALUE) return false;
	memcpy(m_HexAsciiTxMsg,SENDCBI2SOFTVERSION,HEADERSIZE);
	m_HexAsciiTxMsg[HEADERSIZE] = 0;

	SendMessage(m_HexAsciiTxMsg);
	
	// Wait for CBI ACK
	int rCode = GetAnswer(pBuf);
	if (rCode != FACK && rCode != FMSG)
		return false;
	// Get Version number
	rCode = GetAnswer(pBuf);
	if (rCode != FACK && rCode != FMSG)
		return false;

	// Get US flag
	rCode = GetAnswer(&tmp);
	if (rCode != FACK && rCode != FMSG)
		return false;
	if (tmp.Buffer[0]=='0') m_USflag = 0;
	else m_USflag = 1;
	return true;
}

CString CWizardCBI::FindCBIComPort(int *pCom /*=NULL Automatic*/)
{
	// Port is already defined
	CWizardCBI::s_Msg Rx;

	// Port already opened
	if(m_Com.GetComHandle()!=INVALID_HANDLE_VALUE)
	{
		if (IsCBIThere(&Rx))
		{
			if (pCom) 
				*pCom = m_Com.GetPortNumber();
			// buf must be converted to a TCHAR
			return (Byte2TChar(Rx.Buffer,Rx.Length));
		}
		else
		{
			m_Com.PortClose();
			if (pCom) *pCom = 0;
			return (_T(""));
		}
	}
	// Try to find the port where is connected the CBI	
/*	int i,min,max;
	min = 1;
	max = MAXCOMNUMBER;
	if (pCom)
		if (*pCom)
			min=max=*pCom;
	for(i=min;i<max;i++)
*/
	int i;
	CStringArray ComPorts;
	CCom::EnumerateComPortUsingRegistry(ComPorts);

	for (int j=0; j<ComPorts.GetSize(); j++)
	{
		CString str = ComPorts.GetAt(j);
		TRACE(_T("\r\n%s"),(LPCTSTR) str);
		str.Delete(0,3);		// Remove "COM"
		i = _wtoi((LPCTSTR)str);		
		bool bRet = m_Com.PortOpen(i,19200,NOPARITY,8,TWOSTOPBITS);
		if (bRet)
		{
			if (IsCBIThere(&Rx))
			{
				if (pCom) 
					*pCom = i;
				return (Byte2TChar(Rx.Buffer,Rx.Length));
			}
			m_Com.PortClose();
		}
	}
	if (pCom) *pCom=0;
	return (_T(""));
}
/////////////////////////////////////////////////////////////////////////////////////
// Convert a CBI answer (buffer of byte) to a TCHAR buffer
// 
LPCTSTR CWizardCBI::Byte2TChar(BYTE *pBufChar,int MaxLen )
{
	static TCHAR BufTChar[4*BUFFERSIZE];
	TCHAR* pBufTChar=BufTChar;
	while (MaxLen)
	{
		*pBufTChar++=(TCHAR)*pBufChar++;
		MaxLen--;
	}
	return BufTChar;
}

int CWizardCBI::HexAsciiToInt(BYTE *pa, int len)
{
	int val=0;
	for (int i=0; i<len; i++)
	{
		val <<= 8;
		val += HexAsciiToByte(pa);
		pa++;
	}
	return val;
}

/////////////////////////////////////////////////////////////////////////////////////
// Complete checksum for current message and send it to the serial port
//
void CWizardCBI::SendMessage(BYTE *pMsg2CBI)
{
	
	BYTE Tx[520], *pTx;
	// Copy Header
	memcpy(Tx,pMsg2CBI,HEADERSIZE);

	BYTE *pa=pMsg2CBI+1;
	// Get message len ... byte 1 
	int len = HexAsciiToByte(pa);
	
	// Expand data zone 1 byte to 2
	pTx = &Tx[HEADERSIZE];
	pa = &pMsg2CBI[HEADERSIZE];
	int i;
	for (i=0; i<len; i++)
	{
		ByteToHexAscii(*pa, pTx);		
		pa++;
		pTx += 2;
	}
	// Compute checksum without start character
	BYTE checksum=0;
	pTx = &Tx[1];
	for (i=0; i<len+4; i++)
	{
		checksum+=HexAsciiToByte(pTx);	
		pTx+=2;	
	}
	checksum = 256-(checksum&0xFF); 				
	// Set checksum at the correct position;
	ByteToHexAscii(checksum,&Tx[HEADERSIZE+len*2]);	// at the message end 
	// Send Message...
	pTx=Tx;
	ASSERT(m_Com.GetComHandle()!= INVALID_HANDLE_VALUE);
	// Protect ourself against PurgeCom that can be crash OS
	Sleep(10);
	TASApp.PumpMessages();
	m_Com.PurgeRx();
	m_Com.PurgeTx();//m_Com.PurgeCom();
	m_Com.WriteBuffer((const char*) pTx,len*2+11);
}

int CWizardCBI::GetAnswer(s_Msg *pMsg)
{
	BYTE *pBuf = m_RxBuf;
	int count=0;
	memset(m_RxBuf,0,sizeof(m_RxBuf));
	memset(pMsg,0,sizeof(s_Msg));
	DWORD dwTick = GetTickCount();
	bool fTO=false;
	ASSERT(m_Com.GetComHandle()!= INVALID_HANDLE_VALUE);
	while (true)
	{
		if (!m_Com.IsRXEmpty())
		{
			char onechar=0;
			if (m_Com.ReadChar(onechar))
			{
				if (onechar == '!')
					return FACK;
				if (onechar == 0x0A && *(pBuf-1) == 0x0D)
				{
					if (count>1)				// Keep 0x0D character when the message is 0x0D 0x0A
						*(pBuf-1) = 0;
					*pBuf = 0;					// End of string	
					m_Com.WriteChar('!');
					pMsg->Length = count;
					memcpy(pMsg->Buffer,m_RxBuf,count);
					return FMSG;
				}
				*pBuf = onechar;
				count++;
				if (count<sizeof(m_RxBuf))
					pBuf++;
				else			// ERROR
				{
					pBuf = m_RxBuf;			
					return FERROR;
				}
			}
		}
		TASApp.PumpMessages();
		if ((GetTickCount()-dwTick)>1000)		//FULL ANSWER T.O. not intercharacter T.O.
			return FTO;
	}
}
// Return the piping Tab where the uid object is stored
CTable *CWizardCBI::GetPipingTab( UINT uid )
{
	// Piping_Tab by default.
	CTable *pTab = TASApp.GetpTADS()->GetpHydroModTable();

	int Index = uid / MAXVALVES;

	if( Index < m_aTableUID.GetCount() )
	{
		s_TabUid sTabUid = m_aTableUID[Index];
		pTab = (CTable*)( sTabUid.TabIDPtr.MP );
	}

	ASSERT( NULL != pTab );
	return pTab;
}

/////////////////////////////////////////////////////////////////////////////
// CWizardCBI message handlers
/////////////////////////////////////////////////////////////////////////////////////
BEGIN_EVENTSINK_MAP(CWizardCBI, CWizardManager)
END_EVENTSINK_MAP()

LRESULT CWizardCBI::OnInitComPort(WPARAM wParam, LPARAM lParam)
{
	if (m_Com.GetComHandle() != INVALID_HANDLE_VALUE)
	{
		m_Com.PortClose();
	}
	CStringArray ComPorts;
	CCom::EnumerateComPortUsingRegistry(ComPorts);

	for (int i=1; i<MAXCOMNUMBER; i++)
		m_PortAvailable[i-1]=false;

	for (int j=0; j<ComPorts.GetSize(); j++)
	{
		CString str = ComPorts.GetAt(j);
		TRACE(_T("\r\n%s"),(LPCTSTR) str);
		str.Delete(0,3);		// Remove "COM"
		int i = _wtoi((LPCTSTR)str);		
		i--;
		if (i>=0 && i<MAXCOMNUMBER)
		m_PortAvailable[i]=true;
	}

/*	for (int i=1; i<MAXCOMNUMBER; i++)
	{
		m_PortAvailable[i-1]=false;
		bool bRet = m_Com.PortOpen(i,19200,NOPARITY,8,TWOSTOPBITS);
		if (bRet)
		{
			m_PortAvailable[i-1]=true;
			m_Com.PortClose();
		}
	}
*/
	::PostMessage(m_PanelCBI1.m_hWnd,WM_USER_INITCOMPORT,0,0);
	return 0;
}

BOOL CWizardCBI::OnInitDialog() 
{
	CWizardManager::OnInitDialog();

	// Initialize m_aTableUID, contain all table ID with something to send to the CBI
	m_aTableUID.RemoveAll();
	s_TabUid sTabUid;
	sTabUid.FirstUid = m_TabFirstUID.PipingTab;
	sTabUid.TabIDPtr = TASApp.GetpTADS()->GetpHydroModTable()->GetIDPtr();
	m_aTableUID.Add( sTabUid );

	sTabUid.FirstUid = m_TabFirstUID.SSelHub;
	sTabUid.TabIDPtr = TASApp.GetpTADS()->GetpHUBSelectionTable()->GetIDPtr();
	m_aTableUID.Add( sTabUid );

	::PostMessage( m_hWnd, WM_USER_INITCOMPORT, 0, 0);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CPanelCBI1 dialog
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CPanelCBI1::CPanelCBI1( CWnd* pParent )
	: CDlgWizard( CPanelCBI1::IDD, pParent )
{
	m_pclWizardCBI = NULL;
	m_fAreaCompatibility = false;
}

CPanelCBI1::~CPanelCBI1()
{
	m_brWhiteBrush.DeleteObject();	
}

BOOL CPanelCBI1::Create( CWizardManager* pParent )
{
	BOOL fReturn = CDlgWizard::Create( pParent );
	m_pclWizardCBI = (CWizardCBI*)pParent;
	return fReturn;
}

void CPanelCBI1::DoDataExchange(CDataExchange* pDX)
{
	CDlgWizard::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBOPORT, m_ComboPort);
}


BEGIN_MESSAGE_MAP(CPanelCBI1, CDlgWizard)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USER_INITCOMPORT, OnInitComPort)
END_MESSAGE_MAP()

bool CPanelCBI1::OnActivate()
{
	m_pclWizardCBI->SetButtonText(CWizardManager::WizButFinish,IDS_WIZBUT_CANCEL);
	return true;
}

bool CPanelCBI1::OnAfterActivate()
{
	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	SetFocus();
	return true;
}

BOOL CPanelCBI1::OnWizNext()
{
	int comport = m_ComboPort.GetItemData(m_ComboPort.GetCurSel());
	m_pclWizardCBI->m_ButWizNext.EnableWindow(FALSE);
	BeginWaitCursor();
	CString CBIVersion = m_pclWizardCBI->FindCBIComPort(&comport);
	EndWaitCursor();
	if (comport==0)
	{
		CString str;
		str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
		AfxMessageBox((LPCTSTR)str,MB_ICONEXCLAMATION);
		m_pclWizardCBI->m_ButWizNext.EnableWindow(TRUE);
		return false;
	};
	m_pclWizardCBI->m_ButWizNext.EnableWindow(TRUE);

	/* Version string structure
		PRXA.Y STD     for instance "PR3A.1 STD" 
		PRXA.Y UK
		PRXA.Y US
		for identifying version "XA" of the CBI software for the 
		STD, UK or US versions of the DB. 

		The X integer:
		Refer to the major version of the combined CBI software and
		database. They constitute a product modification and require 
		a full reload of the CBI software.

		The A character: 
		Is associated to minor updates of the software and/or 
		database.

		The Y integer after the dot:
		Is a HySelect compatibility index.

		The last 3 characters:
		Are there to know if the DB is the Standard, UK
		or US one. Helps to prevent CBI transfers failing 
		because of unknown valves. */

	// Parsing of the version string
	CString str;
	CBIVersion.TrimLeft();
	CBIVersion.TrimRight();
	// Test on beginning of version number to see if it seems to be a valid one
	if (CBIVersion.Left(2) != CBIVERSIONPREFIX)
	{
		TASApp.AfxLocalizeMessageBox(AFXMSG_CBITOOOLD);
		return false;
	};

	// Extract CBI/TAS2 compatibility index from TADB and CBIVersion
	CTADatabase* pTADB = TASApp.GetpTADB(); ASSERT(pTADB);
	CDB_MultiString *pMS = (CDB_MultiString *)pTADB->Get(_T("CBI_COMPATIBIL")).MP;
	if (!pMS)	// TAS2 DB is too old !!! Should not happen because "CBI-able" TAS2 implies updated DB
	{
		TASApp.AfxLocalizeMessageBox(AFXMSG_TAS2TOOOLD);
		return false;
	};
	long TAS2CompIndex;
	ReadLong(pMS->GetString(0),&TAS2CompIndex);
	long index1 = CBIVersion.Find('.');
	int index2 = CBIVersion.Find(' ',index1);
	str = CBIVersion.Mid(index1+1,index2-index1-1);
	if (ReadLong(str,&index1) != RD_OK)
	{
		TASApp.AfxLocalizeMessageBox(AFXMSG_CBITOOOLD);
		return false;
	};

	// Test on CBI/TAS2 compatibility index
	// Hardcoding: checking of compatibility index between 1 and 2 inhibited
	if ((index1 == 2 && TAS2CompIndex == 1)||(index1 == 1 && TAS2CompIndex == 2));
	else if (index1 > TAS2CompIndex)	// TAS2 is too old
	{
		TASApp.AfxLocalizeMessageBox(AFXMSG_TAS2TOOOLD);
		return false;
	}
	else if (index1 < TAS2CompIndex)	// CBI software is too old
	{
		TASApp.AfxLocalizeMessageBox(AFXMSG_CBITOOOLD);
		return false;
	};

	// Test on DB area compatibility
	m_fAreaCompatibility=true;
	CString TAS2DBAreaComp = pMS->GetString(1);
	str = CBIVersion.Mid(index2+1);	str.TrimLeft();
	if (str != TAS2DBAreaComp)
	{
		
		// Specific treatment for 'EU' DB area compatibility index
		// Since PR3C.1 for European countries, 'EU' replace 'STD'.
		// To take in iCount old CBI version like PR3A.1 & PR3B.1 the following test has been added
		//++++++++++++++++++++
		if (TAS2DBAreaComp==_T("EU")) 
		{
			if (CBIVersion == _T("PR3A.1 STD") || CBIVersion == _T("PR3B.1 STD"))
				return true;
		
			// For version >= PR5A.1 EU has been replaced by STD version
			if (CBIVersion.Mid(7,3) == _T("STD"))
			{
				CString strFig = CBIVersion.Mid(2,1);
				int iFig = _ttoi((LPCTSTR)strFig);
				if (iFig >= 5) return true;
			}
		}
		//++++++++++++++++++++

		CString strMsg;
		FormatString(strMsg,AFXMSG_CBITAS2INCOMPATIBLEDB,(LPCTSTR)TAS2DBAreaComp,(LPCTSTR)str);
		::AfxMessageBox(strMsg);
		m_fAreaCompatibility=false;
		return true;
	};

	return true;
}

LRESULT CPanelCBI1::OnInitComPort(WPARAM wParam, LPARAM lParam)
{
	m_ComboPort.ResetContent();
	CString str;
	str=TASApp.LoadLocalizedString(IDS_PANELCBI1_COMBOPORTAUTO);
	m_ComboPort.AddString(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI1_COMBOPORTCOM);
	for (int i=1; i<MAXCOMNUMBER; i++)
	{
		if (m_pclWizardCBI->IsPortAvailable(i))
		{
			CString str1;
			str1.Format(_T("%s %u"),(LPCTSTR)str,i);
			int j=m_ComboPort.AddString(str1);
			m_ComboPort.SetItemData(j,i);
		}
	}
	m_ComboPort.SetCurSel(0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CPanelCBI1 message handlers

BOOL CPanelCBI1::OnInitDialog() 
{
	CDlgWizard::OnInitDialog();

	// Set the white rectangle to the first drawn window
	GetDlgItem(IDC_STATICRECTANGLE)->BringWindowToTop();

	m_brWhiteBrush.CreateSolidBrush(RGB(255,255,255));

	m_pclWizardCBI = (CWizardCBI *)GetWizMan();
	
	CString str; 
	str=TASApp.LoadLocalizedString(IDS_PANELCBI1_STATICSELECTPORT);
	GetDlgItem(IDC_STATICSELECTPORT)->SetWindowText(str);	
	str=TASApp.LoadLocalizedString(IDS_PANELCBI1_TEXT);
	GetDlgItem(IDC_EDITTEXT)->SetWindowText(str);

	// Modify size of CBI bitmap to take in iCount large font
	CRect rect,LineRect;
	GetDlgItem(IDC_STATICRECTANGLE)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	GetDlgItem(IDC_STATICBMP)->GetWindowRect(&LineRect);
	ScreenToClient(&LineRect);
	LineRect.right = rect.left-1;
	LineRect.bottom = rect.bottom;
	GetDlgItem(IDC_STATICBMP)->MoveWindow(&LineRect);

	m_ComboPort.ResetContent();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CPanelCBI1::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	if (pWnd->m_hWnd == GetDlgItem(IDC_EDITTEXT)->m_hWnd)
	{
		pDC->SetTextColor(RGB(0,0,0));
		pDC->SetBkColor(RGB(255,255,255));
		return (HBRUSH) m_brWhiteBrush;
	}
	
	hbr = CDlgWizard::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CPanelCBI2 dialog
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CPanelCBI2::CPanelCBI2( CWnd* pParent )
	: CDlgWizard( CPanelCBI2::IDD, pParent )
{
	m_pclWizardCBI = NULL;
	m_CheckCollectData = FALSE;
	m_CheckReadProj = FALSE;
	m_CheckSendProj = FALSE;
	m_pTADS = NULL;
	m_strCBIVerComp = _T("");
	CString m_strCBIVersion = _T("");
	m_iCBICompat = -1;
	m_fButIgnoreVisible = false;
	ZeroMemory( &m_strProblemPos, 8 * sizeof( POSITION ) );
}

BOOL CPanelCBI2::Create( CWizardManager* pParent )
{
	BOOL fReturn = CDlgWizard::Create( pParent );
	m_pclWizardCBI = (CWizardCBI*)pParent;
	return fReturn;
}

void CPanelCBI2::DoDataExchange(CDataExchange* pDX)
{
	CDlgWizard::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECKSENDPROJ, m_CheckSendProjBut);
	DDX_Control(pDX, IDC_BUTCANNOTSEND, m_ButCannotSend);
	DDX_Control(pDX, IDC_BUTERASELOGDATA, m_ButEraseLogData);
	DDX_Control(pDX, IDC_BUTERASESAVEDDATA, m_ButEraseSavedData);
	DDX_Control(pDX, IDC_EDITVERSION, m_EditVersion);
	DDX_Control(pDX, IDC_GROUPBOX, m_GroupBox);
	DDX_Check(pDX, IDC_CHECKCOLLECTDATA, m_CheckCollectData);
	DDX_Check(pDX, IDC_CHECKREADPROJ, m_CheckReadProj);
	DDX_Check(pDX, IDC_CHECKSENDPROJ, m_CheckSendProj);
}

BEGIN_MESSAGE_MAP(CPanelCBI2, CDlgWizard)
	ON_BN_CLICKED(IDC_CHECKCOLLECTDATA, OnCheckcollectdata)
	ON_BN_CLICKED(IDC_CHECKREADPROJ, OnCheckreadproj)
	ON_BN_CLICKED(IDC_CHECKSENDPROJ, OnChecksendproj)
	ON_BN_CLICKED(IDC_BUTERASELOGDATA, OnButeraselogdata)
	ON_BN_CLICKED(IDC_BUTERASESAVEDDATA, OnButerasesaveddata)
	ON_BN_CLICKED(IDC_BUTCANNOTSEND, OnButcannotsend)
END_MESSAGE_MAP()

bool CPanelCBI2::OnActivate()
{
	m_pTADS = TASApp.GetpTADS();
	m_strCBIVerComp = m_pclWizardCBI->FindCBIComPort();
	m_strCBIVerComp.TrimLeft();
	m_EditVersion.SetWindowText( m_strCBIVerComp );
	
	m_strCBIVersion = m_strCBIVerComp.Mid( 2, 2 );
	m_iCBICompat = _ttoi( (LPCTSTR)m_strCBIVerComp.Mid( 5, 1 ) );
	
	// To Enable/Disable "Send" check box, verify module integrity and valve compatibility .
	// Initialize columns of DialogListBox.
	m_DlgCannotSend.SetColumn( 5 );
	m_DlgCannotSend.AddHeaderStr( Problem, IDS_DLGCANNOTSEND_PROBLEM );
	m_DlgCannotSend.AddHeaderStr( ModuleName, IDS_DLGCANNOTSEND_MODULE );
	m_DlgCannotSend.AddHeaderStr( ValveName, IDS_DLGCANNOTSEND_VALVENAME );
	m_DlgCannotSend.AddHeaderStr( Valve, IDS_DLGCANNOTSEND_VALVE );
	m_DlgCannotSend.AddHeaderStr( ValveIndex, IDS_DLGCANNOTSEND_VALVEINDEX );
	
	// Fill DlgCannotSend with problem messages.
	m_strProblemPos[enum_ProbDescription::UndefValv] = m_DlgCannotSend.AddStrToList( 0, IDS_DLGCANNOTSEND_UNDEFVALVDESCR, ValveName );
	m_strProblemPos[enum_ProbDescription::EmptyMod] = m_DlgCannotSend.AddStrToList( 0, IDS_DLGCANNOTSEND_EMPTYMODDESCR, ModuleName );
	m_strProblemPos[enum_ProbDescription::NotInCBI] = m_DlgCannotSend.AddStrToList( 0, IDS_DLGCANNOTSEND_NOTINCBIDESCR, ValveName, Valve );
	m_strProblemPos[enum_ProbDescription::UndetValve] = m_DlgCannotSend.AddStrToList( 1, IDS_DLGCANNOTSEND_UNDETVALVDESCR, Valve );
	
	CString str = TASApp.LoadLocalizedString( IDS_DLGCANNOTSEND_DIFWATERCHARPROB );
	str += _T("\r\n\r\n");
	
	CString str1 = TASApp.LoadLocalizedString( IDS_DLGCANNOTSEND_DIFWATERCHARSOL );
	str += str1;
	m_strProblemPos[enum_ProbDescription::TooManyWaterChar] = m_DlgCannotSend.AddStrToList( 0, str );

	int iCount = 1;
	enum enum_CheckProjectRetCode ret, RetCode = ProjectOK;
	
	for( int i = 0; i < m_pclWizardCBI->m_aTableUID.GetCount(); i++ )		// Scan each table PIPING_TAB, SELHUB_TAB,....
	{
		CTable *pTab = (CTable *)m_pclWizardCBI->m_aTableUID.GetAt( i ).TabIDPtr.MP;
		ASSERT( NULL != pTab );
		
		// Verify each module and valve.
		for( IDPTR HMIDPtr = pTab->GetFirst(); _T('\0') != *HMIDPtr.ID; HMIDPtr = pTab->GetNext( HMIDPtr.MP ) )
		{
			ret = CheckProject( (CDS_HydroMod *)HMIDPtr.MP, iCount );

			if( RetCode == ProjectOK )
			{
				RetCode = ret;
			}
			else if( RetCode == ProjectMaybeOK && ret == ProjectNotOK )
			{
				RetCode = ProjectNotOK;
			}
		}
	}
	
	if( RetCode == ProjectNotOK || RetCode == ProjectMaybeOK )
	{
		// Disable check box send.
		m_CheckSendProjBut.EnableWindow( FALSE );
		
		// Unhide button for DLGCANNOTSEND.
		m_ButCannotSend.ShowWindow( SW_SHOW );
	}
	
	m_fButIgnoreVisible = ( RetCode == ProjectMaybeOK ) ? true : false;

	if( false == m_pclWizardCBI->m_PanelCBI1.IsAreaCompatible() )
	{
		// Disable check box send .
		m_CheckSendProjBut.EnableWindow( FALSE );
		
		// Unhide button for DLGCANNOTSEND.
		m_ButCannotSend.ShowWindow( SW_HIDE );
	}

	return true;
}

bool CPanelCBI2::OnAfterActivate()
{
	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	SetFocus();
	return true;
}


// Check the project coherence
// iCount the valve number
// return true if OK
enum CPanelCBI2::enum_CheckProjectRetCode CPanelCBI2::CheckProject( CDS_HydroMod *pHM, int &count )
{
	if( NULL == pHM )
	{
		return ProjectNotOK;
	}

	enum enum_CheckProjectRetCode RetCode = ProjectOK;
	enum enum_ValveInCBI retInCBI;
	CString str;
	CDB_TAProduct *pTAProd = (CDB_TAProduct *)pHM->GetTADBValveIDPtr().MP;
	
	// Verify water characteristic.
	// Take the first water characteristic as a reference.
	if( 1 == count )
	{
		// HYS-1716: In CBI we have only one water characteristic that is the general one.
		// Here we can take the first water of the root (That is finally the same as the global project temperature).
		if( NULL != pHM->GetpInWaterChar( CAnchorPt::PipeLocation_Supply ) )
		{
			*m_pclWizardCBI->GetpWaterChar() = *pHM->GetpInWaterChar( CAnchorPt::PipeLocation_Supply );
		}
	}
	else
	{
		// Test hub and not stations.
		if( false == pHM->IsClass( CLASS( CDS_HmHubStation ) ) && NULL != pHM->GetpInWaterChar( CAnchorPt::PipeLocation_Supply ) )
		{
			if( *pHM->GetpInWaterChar( CAnchorPt::PipeLocation_Supply ) != *m_pclWizardCBI->GetpWaterChar() )
			{
				// Column Problem.
				str=TASApp.LoadLocalizedString( IDS_DLGCANNOTSEND_DIFWATERCHAR );
				m_DlgCannotSend.AddStr( Problem, str );
				
				// Column Module Name.
				str= _T("-");

				if( true == ( pHM->GetIDPtr().PP)->IsHMFamily() )
				{
					str = ( (CDS_HydroMod *)pHM->GetIDPtr().PP )->GetHMName();
				}

				m_DlgCannotSend.AddStr( ModuleName, str );
				
				// Column Valve Name.
				m_DlgCannotSend.AddStr( ValveName, (LPCTSTR)CString( pHM->GetHMName() ) );
				
				// Column Valve.
				if( NULL != pTAProd )		// existing valve
				{
					m_DlgCannotSend.AddStr( Valve, (LPCTSTR)pTAProd->GetName() );
				}
				else
				{
					m_DlgCannotSend.AddStr( Valve, _T("?") );
				}

				// Column Valve Index.
				str.Format( _T("%d"), pHM->GetPos() );
				m_DlgCannotSend.AddStr( ValveIndex, (LPCTSTR)str );

				m_DlgCannotSend.AddLine( (LPARAM)m_strProblemPos[enum_ProbDescription::TooManyWaterChar] );
				RetCode = ProjectNotOK;
			}
		}
	}
	
	if( false == pHM->IsaModule() )								// pHM is a valve
	{
		if( false == pHM->HasaValve() || pHM->GetQDesign() <= 0.0 )	// No valve specified or flow <= 0; 
														// pHM->Getqd() <= 0.0 is useless, already tested in dialog box AddValve
		{
			// Column Problem.
			str = TASApp.LoadLocalizedString( IDS_DLGCANNOTSEND_UNDEFVALV );
			m_DlgCannotSend.AddStr( Problem, str );
			
			// Column Module Name.
			m_DlgCannotSend.AddStr( ModuleName, ( (CDS_HydroMod *)pHM->GetIDPtr().PP )->GetHMName() );
			
			// Column Valve Name.
			m_DlgCannotSend.AddStr( ValveName, (LPCTSTR)CString( pHM->GetHMName() ) );
			
			// Column Valve.
			if( NULL != pTAProd )		// existing valve
			{
				m_DlgCannotSend.AddStr( Valve, (LPCTSTR)pTAProd->GetName() );
			}
			else
			{
				m_DlgCannotSend.AddStr( Valve, _T("?") );
			}

			// Column Valve Index.
			str.Format( _T("%d"), pHM->GetPos() ); 
			m_DlgCannotSend.AddStr( ValveIndex, (LPCTSTR)str );

			m_DlgCannotSend.AddLine( (LPARAM)m_strProblemPos[enum_ProbDescription::UndefValv] );
			RetCode = ProjectNotOK;
		}
		else											// Valve must be tested for the compatibility
		{
			count++;
			retInCBI=pHM->ValveIsInCBI(m_strCBIVersion, m_iCBICompat);
			switch (retInCBI)
			{
				case evi_ValvePresent:
				break;
				case evi_Undetermined:
					// column Problem
					str=TASApp.LoadLocalizedString(IDS_DLGCANNOTSEND_UNDETVALV);
					m_DlgCannotSend.AddStr(Problem,str);
					// column Module Name
					m_DlgCannotSend.AddStr(ModuleName,((CDS_HydroMod*)pHM->GetIDPtr().PP)->GetHMName());
					// column Valve Name
					m_DlgCannotSend.AddStr(ValveName,(LPCTSTR)CString(pHM->GetHMName()));
					// column Valve
					m_DlgCannotSend.AddStr(Valve,CString(pHM->GetCBIType())+CString(pHM->GetCBISize()));
					
					// column Valve Index
					str.Format(_T("%d"),pHM->GetPos()); 
					m_DlgCannotSend.AddStr(ValveIndex,(LPCTSTR)str);

					m_DlgCannotSend.AddLine((LPARAM)m_strProblemPos[enum_ProbDescription::UndetValve]);
					if (RetCode == ProjectOK)
						RetCode = ProjectMaybeOK;
				break;
				case evi_ValveNotPresent:
					// column Problem
					str=TASApp.LoadLocalizedString(IDS_DLGCANNOTSEND_NOTINCBI);
					m_DlgCannotSend.AddStr(Problem,str);
					// column Module Name
					m_DlgCannotSend.AddStr(ModuleName,((CDS_HydroMod*)pHM->GetIDPtr().PP)->GetHMName());
					// column Valve Name
					m_DlgCannotSend.AddStr(ValveName,(LPCTSTR)CString(pHM->GetHMName()));
					// column Valve
					if (pTAProd)		// existing valve
						m_DlgCannotSend.AddStr(Valve,(LPCTSTR)pTAProd->GetName());
					else
						m_DlgCannotSend.AddStr(Valve,_T("-"));

					
					// column Valve Index
					str.Format(_T("%d"),pHM->GetPos()); 
					m_DlgCannotSend.AddStr(ValveIndex,(LPCTSTR)str);

					m_DlgCannotSend.AddLine((LPARAM)m_strProblemPos[enum_ProbDescription::NotInCBI]);
					RetCode = ProjectNotOK;
				break;
			}
		}
	}
	else							// pHM is a module
	{
		if (pHM->HasaValve())		// Partner valve exists
		{
			count++;
			retInCBI=pHM->ValveIsInCBI(m_strCBIVersion, m_iCBICompat);
			switch (retInCBI)
			{
				case evi_ValvePresent:
				break;
				case evi_Undetermined:
					// column Problem
					str=TASApp.LoadLocalizedString(IDS_DLGCANNOTSEND_UNDETVALV);
					m_DlgCannotSend.AddStr(Problem,str);
					// column Module Name
					m_DlgCannotSend.AddStr(ModuleName,((CDS_HydroMod*)pHM->GetIDPtr().PP)->GetHMName());
					// column Valve Name
					m_DlgCannotSend.AddStr(ValveName,(LPCTSTR)CString(pHM->GetHMName()));
					// column Valve
					m_DlgCannotSend.AddStr(Valve,CString(pHM->GetCBIType())+CString(pHM->GetCBISize()));
					
					// column Valve Index
					str.Format(_T("%d"),pHM->GetPos()); 
					m_DlgCannotSend.AddStr(ValveIndex,(LPCTSTR)str);

					m_DlgCannotSend.AddLine((LPARAM)m_strProblemPos[enum_ProbDescription::UndetValve]);
					if (RetCode == ProjectOK)
						RetCode = ProjectMaybeOK;
				break;
				case evi_ValveNotPresent:
					// column Problem
					str=TASApp.LoadLocalizedString(IDS_DLGCANNOTSEND_NOTINCBI);
					m_DlgCannotSend.AddStr(Problem,str);
					// column Module Name
					m_DlgCannotSend.AddStr(ModuleName,((CDS_HydroMod*)pHM->GetIDPtr().PP)->GetHMName());
					// column Valve Name
					m_DlgCannotSend.AddStr(ValveName,(LPCTSTR)CString(pHM->GetHMName()));
					// column Valve
					if (pTAProd)		// existing valve
						m_DlgCannotSend.AddStr(Valve,(LPCTSTR)pTAProd->GetName());
					
					// column Valve Index
					str.Format(_T("%d"),pHM->GetPos()); 
					m_DlgCannotSend.AddStr(ValveIndex,(LPCTSTR)str);

					m_DlgCannotSend.AddLine((LPARAM)m_strProblemPos[enum_ProbDescription::NotInCBI]);
					RetCode = ProjectNotOK;
				break;
			}
		}

		bool bModuleEmpty = false;
		if (pHM->GetQDesign()<=0.0)		// Flow <= 0 --> Module empty
			bModuleEmpty = true;
		// Check if the module contains something and recursively if Children are OK
		bool bOneValve = false;
		for (IDPTR IDPtr=pHM->GetFirst(); *IDPtr.ID; IDPtr=pHM->GetNext(IDPtr.MP))
		{
//			if (((CDS_HydroMod *)IDPtr.MP)->HasaValve()) bOneValve = true; 
			enum enum_CheckProjectRetCode ret=CheckProject((CDS_HydroMod *)IDPtr.MP,count); // Check children
			switch (ret)
			{
				case ProjectOK:
				break;
				case ProjectMaybeOK:
					if (RetCode == ProjectOK)
						RetCode = ProjectMaybeOK;
				break;
				case ProjectNotOK:
					RetCode = ProjectNotOK;
				break;
			}
		}
	};

	return RetCode;
}

/////////////////////////////////////////////////////////////////////////////
// CPanelCBI2 message handlers

BOOL CPanelCBI2::OnInitDialog() 
{
	CDlgWizard::OnInitDialog();
	
	m_pclWizardCBI = (CWizardCBI *)GetWizMan();

	CString str;
	
	str=TASApp.LoadLocalizedString(IDS_PANELCBI2_STATICCBIVERSION);
	GetDlgItem(IDC_STATIC_TACBIVERSION)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI2_STATICPLEASECHOOSE);
	GetDlgItem(IDC_STATICPLEASECHOOSE)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI_SENDPROJ);
	GetDlgItem(IDC_CHECKSENDPROJ)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI_GETPLANT);
	GetDlgItem(IDC_CHECKREADPROJ)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI_COLLECTDATA);
	GetDlgItem(IDC_CHECKCOLLECTDATA)->SetWindowText(str);

	CImageList* pclImgListButton = TASApp.GetpRCImageManager()->GetImageList( CRCImageManager::ILN_Button );		ASSERT ( NULL != pclImgListButton );
	if( NULL != pclImgListButton )
	{
		m_ButEraseSavedData.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_EraseSavedData ) );
		m_ButEraseLogData.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_EraseLogData ) );
		m_ButCannotSend.SetIcon( (HICON)pclImgListButton->ExtractIcon( CRCImageManager::ILB_InfoCannotSend ) );
	}

	// Create a ToolTipCtrl and add a tool tip for each erase button
	m_ToolTip.Create(this,TTS_NOPREFIX);
	CString TTstr;
	TTstr=TASApp.LoadLocalizedString(IDS_LVTOOLTIP_BUTERASESAVEDDATA);
	m_ToolTip.AddToolWindow(GetDlgItem(IDC_BUTERASESAVEDDATA),TTstr);
	TTstr=TASApp.LoadLocalizedString(IDS_LVTOOLTIP_BUTERASELOGDATA);
	m_ToolTip.AddToolWindow(GetDlgItem(IDC_BUTERASELOGDATA),TTstr);

	((CWizardManager *)m_pclWizardCBI)->DisableButtons(CWizardManager::WizButNext);	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPanelCBI2::OnCheckcollectdata() 
{
	UpdateData();
	if (!m_CheckCollectData && !m_CheckReadProj && !m_CheckSendProj) // at least one checked
		((CWizardManager *)m_pclWizardCBI)->DisableButtons(CWizardManager::WizButNext);	
	else
		((CWizardManager *)m_pclWizardCBI)->EnableButtons(CWizardManager::WizButNext);	

}

void CPanelCBI2::OnCheckreadproj() 
{
	UpdateData();
	if (!m_CheckCollectData && !m_CheckReadProj && !m_CheckSendProj) // at least one checked
		((CWizardManager *)m_pclWizardCBI)->DisableButtons(CWizardManager::WizButNext);	
	else
		((CWizardManager *)m_pclWizardCBI)->EnableButtons(CWizardManager::WizButNext);	
}

void CPanelCBI2::OnChecksendproj() 
{
	UpdateData();
	if (!m_CheckCollectData && !m_CheckReadProj && !m_CheckSendProj) // at least one checked
		((CWizardManager *)m_pclWizardCBI)->DisableButtons(CWizardManager::WizButNext);	
	else
		((CWizardManager *)m_pclWizardCBI)->EnableButtons(CWizardManager::WizButNext);	
}

void CPanelCBI2::OnButeraselogdata() 
{
	if (TASApp.AfxLocalizeMessageBox(AFXMSG_ERASELOGDATA,MB_OKCANCEL| MB_ICONEXCLAMATION)==IDCANCEL) return;
	BYTE Txbuf[20];
	CWizardCBI::s_Msg Rx;
	memcpy(Txbuf,ERASELOGGEDDATA,HEADERSIZE);
	m_pclWizardCBI->SendMessage(Txbuf);				
	// Wait Acknowledge from CBI
	if (m_pclWizardCBI->GetAnswer(&Rx) != FACK )
	{
		::PostMessage(m_pclWizardCBI->m_hWnd,WM_USER_INITCOMPORT,0,0); 
		CString str;
		str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
		AfxMessageBox((LPCTSTR)str);
	}
	else
		TASApp.AfxLocalizeMessageBox(AFXMSG_LOGDATAERASED);

}

void CPanelCBI2::OnButerasesaveddata() 
{
	if (TASApp.AfxLocalizeMessageBox(AFXMSG_ERASESAVEDDATA,MB_OKCANCEL| MB_ICONEXCLAMATION)==IDCANCEL) return;
	BYTE Txbuf[20];
	CWizardCBI::s_Msg Rx;
	memcpy(Txbuf,ERASESAVEDDATA,HEADERSIZE);
	m_pclWizardCBI->SendMessage(Txbuf);				
	// Wait Acknowledge from CBI
	if (m_pclWizardCBI->GetAnswer(&Rx) != FACK )
	{
		::PostMessage(m_pclWizardCBI->m_hWnd,WM_USER_INITCOMPORT,0,0); 
		CString str;
		str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
		::AfxMessageBox((LPCTSTR)str);
	}
	else
		TASApp.AfxLocalizeMessageBox(AFXMSG_SAVEDDATAERASED);
}

void CPanelCBI2::OnButcannotsend() 
{
	m_DlgCannotSend.Sort(Problem);
	int retc = m_DlgCannotSend.Display(IDS_DLGCANNOTSEND_CAPTION,IDS_DLGCANNOTSEND_EDIT1,IDS_DLGCANNOTSEND_STATIC2,
										IDB_CANNOTSENDSIGNS,m_fButIgnoreVisible);
	if (retc == IDIGNORE)		// Enable check box send 
		m_CheckSendProjBut.EnableWindow(TRUE);
	else						// Disable and uncheck check box send 
	{
		m_CheckSendProjBut.SetCheck(FALSE);
		m_CheckSendProjBut.EnableWindow(FALSE);
	}
}

BOOL CPanelCBI2::OnWizCancel()
{
	::PostMessage( m_pclWizardCBI->m_hWnd, WM_USER_INITCOMPORT, 0, 0 ); 
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// CPanelCBI3 dialog
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

CPanelCBI3::CPanelCBI3( CWnd* pParent )
	: CDlgWizard( CPanelCBI3::IDD, pParent )
{
	m_pTADS = NULL;
	m_pUnitDB = NULL;
	m_pclWizardCBI = NULL;
	m_iDTState = -1;
	m_bUserAborted = false;
}

CPanelCBI3::~CPanelCBI3()
{
	m_brWhiteBrush.DeleteObject();	
}

BOOL CPanelCBI3::Create( CWizardManager* pParent )
{
	BOOL fReturn = CDlgWizard::Create( pParent );
	m_pclWizardCBI = (CWizardCBI*)pParent;
	return fReturn;
}

void CPanelCBI3::DoDataExchange(CDataExchange* pDX)
{
	CDlgWizard::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITINFO, m_EditInfo);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
}


BEGIN_MESSAGE_MAP(CPanelCBI3, CDlgWizard)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USER_DATATRANSFERREADY, OnDataTransferReady)
END_MESSAGE_MAP()

// iCount the valve number
void CPanelCBI3::CountValves(CDS_HydroMod *pHM,int &count)
{
	if (!pHM) return ;
	if (!pHM->IsaModule())			// pHM is a valve
	{
		if (pHM->HasaValve())
			count++;
	}
	else							// pHM is a module
	{
		if (pHM->HasaValve())		// Partner valve exists
			count++;
		// Check if the module contains something and recursively if Children are OK
		for (IDPTR IDPtr=pHM->GetFirst(); *IDPtr.ID; IDPtr=pHM->GetNext())
			CountValves((CDS_HydroMod *)IDPtr.MP,count); // Check children
	};
}

// Copy a string from source to target, fill empty space (difference between sizeof(src) and len with space)
// and if fNTS = true put a 0 at the end of the Trg 
void CPanelCBI3::CopyTextWithSpace(BYTE *pTrg, const TCHAR *pSrc, BYTE len,bool fNTS /*=true*/, int fillchar /*=0x20*/ )
{
	ASSERT(pTrg&&pSrc);
	BYTE *pTrgSav = pTrg;
//	memset( pTrg, fillchar, len );
	for( int i = len; i > 0; --i )
	{
		*pTrg = fillchar;
		pTrg++;
	}
	pTrg = pTrgSav;
	int len2cpy = __min(_tcslen(pSrc), len);
	for(int i=0; i<len2cpy; i++)
	{
		*pTrg=(BYTE)*pSrc;
		pTrg++;
		pSrc++;
	}
	if (fNTS)
		pTrgSav[len-1] = 0;
}

bool CPanelCBI3::AddValve(CDS_HydroMod* pHM, BYTE *pTx, int &ProgressPos)
{
	CString str;
	if (!pHM) return false;
	m_Progress.SetPos(ProgressPos++);
	memcpy(pTx,ADDVALVE,HEADERSIZE);
	CDB_TAProduct *pTAProd = (CDB_TAProduct *)pHM->GetTADBValveIDPtr().MP;		// Exist in TADB
	// Test US flag to determine if Normal or alternative names will be sent
	//if (!pTAProd) return false;
	if (!m_pclWizardCBI->GetUSflag())
		pTx[0+HEADERSIZE] = 0;
	else
		pTx[0+HEADERSIZE] = 1;

	// Valve ID
	pTx[1+HEADERSIZE] = pHM->GetUid() >> 8;		//LSB
	pTx[2+HEADERSIZE] = pHM->GetUid() & 0xFF;	//MSB
	// Valve Number
	pTx[3+HEADERSIZE] = (BYTE) pHM->GetPos();
	// Valve Name
	str += pHM->GetHMName();
	CopyTextWithSpace(&pTx[4+HEADERSIZE], (LPCTSTR)str, 11,true);

	if (pTAProd)
	{
		// Valve Type
		CopyTextWithSpace(&pTx[15+HEADERSIZE], pTAProd->GetCBIType(), 12,false);
		// Valve Size
		if (!m_pclWizardCBI->GetUSflag())
			CopyTextWithSpace(&pTx[27+HEADERSIZE], pTAProd->GetCBISize(), 6,false);
		else
			CopyTextWithSpace(&pTx[27+HEADERSIZE], pTAProd->GetCBISizeInch(), 6,false);
	}
	else if (pHM->GetVDescrType() == edt_KvCv)
	{
		// Valve Type
		if (!m_pclWizardCBI->GetUSflag())
			CopyTextWithSpace(&pTx[15+HEADERSIZE], CBIKVVALVETYPE, 12,false);
		else
			CopyTextWithSpace(&pTx[15+HEADERSIZE], CBICVVALVETYPE, 12,false);
		// Valve Size
		CopyTextWithSpace(&pTx[27+HEADERSIZE], CBIKVCVVALVESIZE, 6,false);
	}
	else if (pHM->GetVDescrType() == edt_CBISizeValve || pHM->GetVDescrType() == edt_CBISizeInchValve)
	{
		// Use CBI Type and Size from HydroMod because an unresolved partially defined valve 
		// (i.e. valve defined into the CBI) must be sent back as is to the CBI
		CopyTextWithSpace(&pTx[15+HEADERSIZE], pHM->GetCBIType(), 12,false);
		// Valve Size
		CopyTextWithSpace(&pTx[27+HEADERSIZE], pHM->GetCBISize(), 6,false);
	}
	else // Straight pipes?
	{
		return false;
	};
	// Design flow
	// Converted to l/h
	UnitDesign_struct ud = m_pUnitDB->GetUnit(_U_FLOW,4);
	double	dVal = pHM->GetQDesign() / GetConvOf(ud);
	CString TmpBuf;
	TmpBuf.Format(_T("%.2f"),dVal);
	CopyTextWithSpace(&pTx[33+HEADERSIZE], (LPCTSTR)TmpBuf, 11,true);

	// Valve setting or KvCv value
	//memset (&pTx[44+HEADERSIZE],0,12);
	if (pHM->GetVDescrType()==edt_KvCv)	// KvCv value
	{
		// Kv or Cv
		// Converted to Kv
		UnitDesign_struct ud = m_pUnitDB->GetUnit(_C_KVCVCOEFF,0);
		double dVal = pHM->GetKvCv() / GetConvOf(ud);
		TmpBuf.Format(_T("%.2f"),dVal);
		CopyTextWithSpace(&pTx[44+HEADERSIZE], (LPCTSTR)TmpBuf, 11,true);
	}
	else								// Valve presetting
	{
		double dVal = pHM->GetPresetting();
		if (dVal < 0.0) dVal = 0.0;
		TmpBuf.Format(_T("%.2f"),dVal);
		CopyTextWithSpace(&pTx[44+HEADERSIZE], (LPCTSTR)TmpBuf, 11,true);
	}
	return true;	
}	

//Send current pHM module and all valve included
bool CPanelCBI3::SendModule(CDS_HydroMod* pHM, int &ProgressPos)
{
	BYTE Txbuf[256];
	CWizardCBI::s_Msg Rx;
	if (!pHM) return false;
	CString str; str=TASApp.LoadLocalizedString(IDS_MODULE);
	str += _T(": ") + pHM->GetHMName();
	m_EditInfo.SetWindowText(str);
	// ADD Module
	memcpy(Txbuf,ADDMODULE,HEADERSIZE);
	CopyTextWithSpace(&Txbuf[HEADERSIZE], (LPCTSTR)pHM->GetHMName(), 11,true);
	//WaitMillisec(100);
	m_pclWizardCBI->SendMessage(Txbuf);				// Send Ghost Module
	// Wait Acknowledge from CBI
	if (m_pclWizardCBI->GetAnswer(&Rx) != FACK )
		return false;
	// Add sorted Valves
	CRank ValveList;
	str = _T("");
	IDPTR HMIDPtr = _NULL_IDPTR;
	for (HMIDPtr = pHM->GetFirst();*HMIDPtr.ID;HMIDPtr = pHM->GetNext())
	{
		CDS_HydroMod *pChildHM = dynamic_cast<CDS_HydroMod *>(HMIDPtr.MP);	ASSERT(pChildHM);
		if (pChildHM)
			ValveList.Add(str,((CDS_HydroMod*) HMIDPtr.MP)->GetPos(),(LPARAM)(void *)HMIDPtr.MP);
	}

	LPARAM itemdata;
	for (BOOL flag = ValveList.GetFirst(str,itemdata);flag; flag = ValveList.GetNext(str,itemdata))
	{
		if (AddValve((CDS_HydroMod*) itemdata, Txbuf, ProgressPos))
		{
			m_pclWizardCBI->SendMessage(Txbuf);				// Send Valve description
			// Wait Acknowledge from CBI
			if (m_pclWizardCBI->GetAnswer(&Rx) != FACK )
				return false;
		}
	};
	// Scan for sub module
	for (HMIDPtr = pHM->GetFirst();*HMIDPtr.ID;HMIDPtr = pHM->GetNext())
	{
		if (((CDS_HydroMod*) HMIDPtr.MP)->IsaModule())
			if (!SendModule((CDS_HydroMod*) HMIDPtr.MP, ProgressPos))
				return false;
	};
	return true;
}

// Send the current project to the CBI
//		Return -2	in case of Project problem
//		Return -1	in case of "ERROR" message from CBI
//		Return 0	in case of transmission error
//		Return 1	if OK
int CPanelCBI3::SiteBuilder()
{
	CDS_ProjectParams *m_pPrjParam = m_pTADS->GetpProjectParams();
	ASSERT( NULL != m_pPrjParam );

	CStringArray arStr;

	int count = 1;

	for (int i=0; i<m_pclWizardCBI->m_aTableUID.GetCount(); i++)		// Scan each table PIPING_TAB, SELHUB_TAB,....
	{
		CTable *pTab = (CTable*)m_pclWizardCBI->m_aTableUID.GetAt(i).TabIDPtr.MP; ASSERT (pTab);
		m_pPrjParam->CompleteUID(pTab);
		// Check the project contents, check each root module and iCount the valve number
		IDPTR HMIDPtr = _NULL_IDPTR;
		for (HMIDPtr = pTab->GetFirst();*HMIDPtr.ID;HMIDPtr = pTab->GetNext())
			CountValves((CDS_HydroMod *)HMIDPtr.MP,count);
	}	
	// Set the progress bar range to the number of valve to send
	m_Progress.SetRange(0,count);

	// Create  the Site	command 0x90
	BYTE Txbuf[256];
	CWizardCBI::s_Msg Rx;
	CString TmpBuf;
	memcpy(Txbuf,ADDSITE,HEADERSIZE);
	Txbuf[HEADERSIZE]=1;					// 1 l/h;

	// Test string from JOJ...
/*	CopyTextWithSpace(&Txbuf[HEADERSIZE+1], "test2 ", 21,true,0);
	CopyTextWithSpace(&Txbuf[HEADERSIZE+22],"PROPYL_GLYC", 12, true,0);
	CopyTextWithSpace(&Txbuf[HEADERSIZE+34],"-5.1", 11, true);	
	CopyTextWithSpace(&Txbuf[HEADERSIZE+45],"0.28", 11, true);	*/

//	CDS_ProjectParams *pGP = m_pTADS->GetpProjectParams();
//	ASSERT( NULL != pGP );

	CopyTextWithSpace( &Txbuf[HEADERSIZE + 1], m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name ), 21,true, 0 );

	CWaterChar *pWC = m_pclWizardCBI->GetpWaterChar();
	CString AdditID = pWC->GetAdditID();
	if (AdditID == _T("WATER"))
	{
		CopyTextWithSpace(&Txbuf[HEADERSIZE+22],(LPCTSTR)AdditID, 12, true);
		TmpBuf.Format(_T("%.2f"),pWC->GetTemp());
		CopyTextWithSpace(&Txbuf[HEADERSIZE+34],(LPCTSTR)TmpBuf, 11, true);	
		TmpBuf.Format(_T("%.2f"),0);
		CopyTextWithSpace(&Txbuf[HEADERSIZE+45],(LPCTSTR)TmpBuf, 11, true);	
	}
	else if (AdditID == _T("PROPYL_GLYC") || AdditID == _T("ETHYL_GLYC"))
	{
		CopyTextWithSpace(&Txbuf[HEADERSIZE+22],(LPCTSTR)AdditID, 12, true);
		TmpBuf.Format(_T("%.2f"),pWC->GetTemp());
		CopyTextWithSpace(&Txbuf[HEADERSIZE+34],(LPCTSTR)TmpBuf, 11, true);	
		TmpBuf.Format(_T("%.3f"),pWC->GetPcWeight()/100);
		CopyTextWithSpace(&Txbuf[HEADERSIZE+45],(LPCTSTR)TmpBuf, 11, true);	
	}
	else	// ALC., BRINES, OTHER
	{
		CopyTextWithSpace(&Txbuf[HEADERSIZE+22],_T("OTHER_FLUID"), 12, true);
		TmpBuf.Format(_T("%.3f"),pWC->GetDens());
		CopyTextWithSpace(&Txbuf[HEADERSIZE+34],(LPCTSTR)TmpBuf, 11, true);	
		double dVal = pWC->GetKinVisc();
		UnitDesign_struct ud = m_pUnitDB->GetUnit(_U_KINVISCOSITY,4);			// cStks				
		dVal = (dVal-GetOffsetOf(ud))/GetConvOf(ud);
		TmpBuf.Format(_T("%.3f"),dVal);
		CopyTextWithSpace(&Txbuf[HEADERSIZE+45],(LPCTSTR)TmpBuf, 11, true);	
	}
	
	m_pclWizardCBI->SendMessage(Txbuf);						// Send Question
	// Wait for ACK
	int rCode = m_pclWizardCBI->GetAnswer(&Rx);
	if (rCode != FACK) 
		return false;
	// Wait for a blank row or ERROR
	rCode = m_pclWizardCBI->GetAnswer(&Rx);
	if (rCode != FMSG )
	{
		return false;
	}
	else
	{
		const BYTE error[]={0x45,0x52,0x52,0x4F,0x52};		// "ERROR"	
		if (!memcmp(Rx.Buffer,error,sizeof(error)))
		{
			return -1;
		}
	}
	int ProgressPos = 1;

	// Sort valve list
	CRank ValveList;
	CString	str = _T("");
	for (int i=0; i<m_pclWizardCBI->m_aTableUID.GetCount(); i++)		// Scan each table PIPING_TAB, SELHUB_TAB,....
	{
		CTable *pTab = (CTable*)m_pclWizardCBI->m_aTableUID.GetAt(i).TabIDPtr.MP; ASSERT (pTab);
		IDPTR HMIDPtr = _NULL_IDPTR;
		for (HMIDPtr = pTab->GetFirst();*HMIDPtr.ID;HMIDPtr = pTab->GetNext())
			ValveList.Add(str,((CDS_HydroMod*) HMIDPtr.MP)->GetPos(),(LPARAM)(void *)HMIDPtr.MP);
	};

	LPARAM itemdata;
	//Loop on ROOT Modules
	for (BOOL flag = ValveList.GetFirst(str,itemdata);flag; flag = ValveList.GetNext(str,itemdata))
	{
		CDS_HydroMod *pHM = (CDS_HydroMod*) itemdata;	ASSERT(pHM);
		// If partner valve of pHM exists, send a ghost module with this valve...
		if (*pHM->GetTADBValveIDPtr().ID || pHM->GetVDescrType() == edt_KvCv || 
			pHM->GetVDescrType() == edt_CBISizeValve || pHM->GetVDescrType() == edt_CBISizeInchValve)
		{
			// "Create" a ghost module with a unique name
			memcpy(Txbuf,ADDMODULE,HEADERSIZE);
			CString str; str=TASApp.LoadLocalizedString(IDS_PV);
			str += pHM->GetHMName();
			if (str.GetLength() > CBIMAXMODULENAMECHAR) // Name with added "PV_" is longer than max char. nr for module names
			{ 
				// function m_pTADS->DifferentiateHMName(pTab,str) can't be used because the PV_xxxx%A names are 
				// created temporary and not stored into the TADS,  DifferentiateHMName can't retrieve name already
				// attribued and return always the same PV_xxxxx%A name.
				// Name will be modified by making the last three characters to be
				// "%AA" where AA is incremented until a non-existing name is found
				const int InitPos = CBIMAXMODULENAMECHAR-1-2;
				CString tmpstr;
				bool bUnique = false;
				int LongNameIndex = 0; 
				while(!bUnique)
				{
					str = str.Left(InitPos) + CString('%');
					CString indexstr = Base26Convert(LongNameIndex);
					str += indexstr;
					CDS_HydroMod* pHM = NULL;
					for (int i=0; i<m_pclWizardCBI->m_aTableUID.GetCount()&&!pHM; i++)		// Scan each table PIPING_TAB, SELHUB_TAB,....
					{
						CTable *pTab = (CTable*)m_pclWizardCBI->m_aTableUID.GetAt(i).TabIDPtr.MP; ASSERT (pTab);
						pHM = m_pTADS->FindHydroMod(str, pTab);
					}
					if (NULL == pHM) 
					{
						bUnique = true;
						for (int k=0; k<arStr.GetCount(); k++)
						{
							if (arStr.GetAt(k) == str)
							{
								bUnique = false;
								break;
							}
						}
					}
					LongNameIndex++;
				};
				arStr.Add(str);
			};
			CopyTextWithSpace(&Txbuf[HEADERSIZE], (LPCTSTR)str, 11,true);
		
			m_pclWizardCBI->SendMessage(Txbuf);				// Send Ghost Module
			rCode = m_pclWizardCBI->GetAnswer(&Rx);
			// Wait Acknowledge from CBI
			if (rCode != FACK )
				return false;

			// Send ghost module partner valve
			//str=TASApp.LoadLocalizedString(IDS_PARTNER);
			//str += _T(" ");
			int indexsave = pHM->GetPos();
			pHM->SetPos(1);							// Index of PV in ghost module is always 1
			if (!AddValve(pHM,Txbuf,ProgressPos)) return false;
			pHM->SetPos(indexsave);
			m_pclWizardCBI->SendMessage(Txbuf);				// Send Valve description
			// Wait Acknowledge from CBI
			int rCode = m_pclWizardCBI->GetAnswer(&Rx);
			if (rCode != FACK )
				return false;
		}
		
		// Send module pHM
		if (!SendModule(pHM,ProgressPos)) return false;
	}
	return true;
}

void CPanelCBI3::DeleteTmpSavedData()
{
	CTable *pTab = (CTable *)m_pTADS->Get( _T("TMPSAVEDATA_TAB") ).MP;					ASSERT( pTab );
	
	// Clear all object from the temporary tab.
	for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; )
	{
		// if it's a table remove children...
		if( true == IDPtr.MP->IsClass( CLASS( CTableHM) ) )
		{
			for( IDPTR IDPtrChild = ( (CTable *)IDPtr.MP )->GetFirst( CLASS( CDB_MultiString ) ); *IDPtrChild.ID; )
			{
				IDPTR IDPtrChildNext = ( (CTable *)IDPtr.MP )->GetNext();
				( (CTable *)IDPtr.MP )->Remove( IDPtrChild );
				m_pTADS->DeleteObject( IDPtrChild );
				IDPtrChild = IDPtrChildNext;
			}
		}
		IDPTR IDPtrNext = pTab->GetNext();
		pTab->Remove( IDPtr );
		m_pTADS->DeleteObject( IDPtr );
		IDPtr = IDPtrNext;
	}
}

// Collect all saved data from the CBI
// Save these data into the Temporary SaveDataTab
bool CPanelCBI3::CollectSavedData()
{
	try
	{
		bool bDebug =  TASApp.IsDebugComActif();

		// Initialise project directory.
		CString strPrjDir = GetProjectDirectory();
		CString strDebugTxtFile = strPrjDir + CString( _T("DebugSavedData.txt") );

		CTable *pTab = (CTable *)( m_pTADS->Get( _T("TMPSAVEDATA_TAB") ).MP );
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'TMPSAVEDATA_TAB' table from the datastruct.") );
		}

		DeleteTmpSavedData();
	
		CWizardCBI::s_Msg buf;
		memcpy(buf.Buffer,SENDSAVEDDATA,HEADERSIZE);
		const BYTE barm3h[]={0x33,0x30,0x33,0x30,0x00};		//"3030" 30: bar; 30: m³/h
		memcpy (&buf.Buffer[HEADERSIZE],barm3h,sizeof(barm3h));		//
		m_pclWizardCBI->SendMessage(buf.Buffer);						// Send Question
	
		int row = 1;
		int rCode = FNONE;

		// Reset strDebugTxtFile if needed
		CFileTxt f;

		if( true == bDebug )	
		{
			f.Open( strDebugTxtFile, CFile::modeCreate | CFile::modeWrite );
			
			BYTE tmpbuf[] = { 0xff, 0xfe };
			f.Write( tmpbuf, 2 );
			f.WriteTxtLine( _T("\n") );
		}

		// Get Header.
		while( rCode != FTO && rCode != FERROR && buf.Buffer[0] != 0x0D )
		{
			rCode = m_pclWizardCBI->GetAnswer( &buf );

			if( true == bDebug )
			{
				CString str;
				str.Format( _T("row[%d] : %s\n"), row, m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length ) );
				f.WriteTxtLine( (LPCTSTR)str );
			}
			
			row++;
		}
		
		row = 18;								// Header must be shorter than 18 rows
												// so at the end of header fix the row to 18
		CString strInfoLine1;
		m_EditInfo.SetWindowText(_T(""));
		IDPTR IDPtrPlant = _NULL_IDPTR;
		IDPTR IDPtrPlantTab = _NULL_IDPTR;
		IDPTR IDPtrValve = _NULL_IDPTR;

		CDB_MultiString *pPlant=NULL;
		CDB_MultiString *pValve=NULL;
		CString	sPvP;
		int range = 0;
		
		while( rCode != FTO && rCode != FERROR )
		{
			// Test if user pressed finish button.
			if( true == m_bUserAborted )
			{
				if( IDYES == TASApp.AfxLocalizeMessageBox( IDS_PANELCBI_ABORTCOM, MB_YESNO ) )
				{
					m_iDTState = 0;
					m_pclWizardCBI->Finish();
					return true;
				}
				else
				{
					m_bUserAborted = false;
				}
			}

			rCode = m_pclWizardCBI->GetAnswer( &buf );
			
			if( FTO == rCode || FERROR == rCode )
			{
				break;
			}
		
			if( true == bDebug )
			{
				CString str;
				str.Format( _T("row[%d] : %s\n"), row, m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length ) );
				f.WriteTxtLine( (LPCTSTR)str );
			}
			
			if( 41 == row )
			{ 
				if( 0x0D == buf.Buffer[0] )			// End of Reference next row is Row 24 new Reference
				{
					row = 24;
					continue;
				}
				else						// End of Valve next Row is Row 25 new valve
				{
					row = 25;
				}
			//		continue;		not a continue here because the buf content must evaluate in function of the new row value
			}
			
			if( 18 == row )
			{
				if( 0x0D == buf.Buffer[0] )			// End of Transmission
				{
					break;
				}

				/* New Plant
					into TMPSAVEDATA_TAB
					Create a new CDB_Multistring with a plant description
						line 0 : ID of associated CTable that contains all valve
						line 1 : flagUS
						line 2 : Plant Name
						line 3 : Date
						line 4 : time
						line 5 : Media type
						line 6 : Media temp. alt Media density
						line 7 : Media conc. alt Media visc.
				  
					Create a New CTable with a CDB_MultiString for each valve of the plant
					Valve description:
						line 0:		Partner valve pressure
						line 1:		Reference name / Module Name
						line 2:		Valve ID / UID	
						line 3:		Valve Number / Valve Index
						line 4:		Valve Name
						line 5:		Valve Type Name / STAD,STAM,...
						line 6:		Valve Size	
						line 7:		Desired flow
						line 8:		Q final measurement corresponding to calc. pos.
						line 9:		P final measurement corresponding to calc. pos.
						line 10:	Actual P closed TA BAlance
						line 11:	Actual Pos. TA Balance
						line 12:	Calculated Pos.
						line 13:	P TA Balance corresponding to 11
						line 14:	Q TA Balance corresponding to 11
						line 15:	Kv Or Cv valve
						line 16:	Valve type
				*/
			
				m_pTADS->CreateObject( IDPtrPlant, CLASS( CDB_MultiString ) );
				pTab->Insert( IDPtrPlant );

				pPlant = (CDB_MultiString *) IDPtrPlant.MP;
			
				m_pTADS->CreateObject( IDPtrPlantTab, CLASS( CTable ) );
				pTab->Insert( IDPtrPlantTab );
			
				// Invalidate valve pointer...
				pValve = NULL;
			
				// Set Tab ID.
				pPlant->SetString(0,IDPtrPlantTab.ID);
			
				// Set US flag.
				pPlant->SetString(1,m_pclWizardCBI->GetUSflag()?_T("1"):_T("0"));
			
				// Set Plant Name.
				pPlant->SetString(2,m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length));			
				row++;
				continue;
			}
			
			if( row >= 19 && row <= 23 )
			{
				if( NULL == pPlant )
				{
					return false;	
				}

				pPlant->SetString( row-  16, m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length ) );
				row++;
				continue;
			}
			
			if( 24 == row )	// New Reference, store Pv pressure temporary, the new valve entry will be created in row 25
			{
				if( 0x0D == buf.Buffer[0] )			// End of Plant next row is Row 18 new Plant
				{
					row = 18;
					continue;
				}
				
				sPvP = m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length );
				
				// A new valve must be created.
				pValve = NULL;
				row++;
				
				// Range = 0;
				m_Progress.SetRange( 0, 100 );
				m_Progress.SetPos( 0 );
				continue;
			}
			
			if( 25 == row )	// New Valve, create a new CDB_MultiString in PlantTab
			{
				if( 0x0D == buf.Buffer[0] )			// End of Plant next row is Row 18 new Plant
				{
					row = 24;
					continue;
				}
			
				m_Progress.SetPos( 0 );
			
				// Check if PlantTab exist	
				if( _T('\0') == *IDPtrPlantTab.ID )
				{
					return false;	
				}

				m_pTADS->CreateObject( IDPtrValve, CLASS( CDB_MultiString ) );
				( (CTable *)IDPtrPlantTab.MP )->Insert( IDPtrValve );

				pValve = (CDB_MultiString *)( IDPtrValve.MP );
				pValve->SetString( 0, (LPCTSTR)sPvP );	
				
				// Set Reference Name.
				pValve->SetString( 1, m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length ) );
				
				// Precompose the 1st line of the Edit box.
				FormatString( strInfoLine1, IDS_COLLECTINGDATA, (CString)pPlant->GetString( 2 ), (CString)pValve->GetString( 1 ) );
				row++;
				continue;
			}
			
			if( row >= 26 && row <= 40 )
				/*
				case 26://	line 2:		Valve ID
				case 27://	line 3:		Valve Number
				case 28://	line 4:		Valve Name
				case 29://	line 5:		Valve Type Name
				case 30://	line 6:		Valve Size	
				case 31://	line 7:		Desired flow
				case 32://	line 8:		Q final measurement corresponding to calc. pos.
				case 33://	line 9:		P final measurement corresponding to calc. pos.
				case 34://	line 10:	Actual P closed TA BAlance
				case 35://	line 11:	Actual Pos. TA Balance
				case 36://	line 12:	Calculated Pos.
				case 37://	line 13:	P TA Balance corresponding to 11
				case 38://	line 14:	Q TA Balance corresponding to 11
				case 39://	line 15:	Kv Or Cv valve
				case 40://	line 16:	Valve type
				*/
			{
				if( NULL == pValve )
				{
					return false;
				}

				CString strbuf = m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length );
				pValve->SetString( row - 24, (LPCTSTR)strbuf );

				if( 27 == row )
				{	
					CString str, str1;
					str1.Format( _T("%d"), _tcstol( (LPCTSTR)buf.Buffer, 0, 10 ) );
					FormatString( str, IDS_XVALVESRECEIVED, str1 );
					str = strInfoLine1 + (CString)_T("\r\n") + str; 
					m_EditInfo.SetWindowText( str );
				}
				
				if( 28 == row )
				{
					m_Progress.SetPos( 25 );
				}

				if( 31 == row )
				{
					m_Progress.SetPos( 50 );
				}

				if( 35 == row )
				{
					m_Progress.SetPos( 75 );
				}

				if( 40 == row )
				{
					m_Progress.SetPos( 100 );
				}

				row++;
				continue;
			}
		}
		
		m_EditInfo.SetWindowText( _T("") );
	
		if( true == bDebug )
		{
			f.Close();
		}

		if( FTO == rCode || FERROR == rCode )
		{
			return false;
		}
		
		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCBI3::CollectSavedData'."), __LINE__, __FILE__ );
		throw;
	}
}

// Collect all logged data from the CBI
// Save Data into the Temporary LogData Tab
bool CPanelCBI3::CollectLoggedData()
{
	try
	{
		bool bDebug =  TASApp.IsDebugComActif();
	
		CString strPrjDir = GetProjectDirectory();
		CString strDebugTxtFile = strPrjDir + CString( _T("DebugLoggedData.txt") );

		CDS_LoggedData *pLD;
		CTable *pTab = (CTable *)( m_pTADS->Get( _T("TMPLOGDATA_TAB") ).MP );
		
		if( NULL == pTab )
		{
			HYSELECT_THROW( _T("Internal error: can't retrieve the 'TMPLOGDATA_TAB' table from the datastruct.") );
		}

		// Clear all CDS_LoggedData from the temporary tab.
		IDPTR IDPtr = _NULL_IDPTR;

		for( IDPtr = pTab->GetFirst(); _T('\0') != *IDPtr.ID; )
		{
			IDPTR IDPtrNext = pTab->GetNext();
			pTab->Remove( IDPtr );
			m_pTADS->DeleteObject( IDPtr );
			IDPtr = IDPtrNext;
		}

		// Store all CDS_LoggedData into a temporary tab

		CWizardCBI::s_Msg buf;
		memcpy(buf.Buffer,SENDLOGGEDDATA,HEADERSIZE);
		const BYTE barm3hC[]={0x33,0x30,0x33,0x30,0x33,0x30,0x00};		//"303030" 30: bar; 30: m³/h; 30: Celcius
		memcpy (&buf.Buffer[HEADERSIZE],barm3hC,sizeof(barm3hC));		//
																	// !!! Units are hard coded in CBI for transfer
		m_pclWizardCBI->SendMessage(buf.Buffer);							// Send Question
	
		int row = 1;
		int rCode = FNONE;
	
		CFileTxt f;

		if( true == bDebug )
		{
			f.Open( strDebugTxtFile, CFile::modeCreate | CFile::modeWrite );
			
			BYTE tmpbuf[] = {0xff,0xfe};
			f.Write( tmpbuf, 2 );
			f.WriteTxtLine( _T("\n") );
		}

		// Get Header
		// Header
		while( rCode != FTO && rCode != FERROR && buf.Buffer[0] != 0x0D )
		{
			rCode = m_pclWizardCBI->GetAnswer( &buf );

			if( true == bDebug )
			{
				CString str;
				str.Format( _T("row[%d] : %s\n"), row, m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length ) );
				f.WriteTxtLine( (LPCTSTR)str );
			}
		
			row++;
		}
		
		row = 14;											// Header must be shorter than 14 bytes
															// so at the end of header fix the row to 14
		// In case of there is no data into the CBI
		// we will receive Header follewed in row14 buf == "\r\n" and in row15 buf == "\r\n", so store site name temporary
		// and do the test in row 15
		CString strTempBuf;
		int iLog=0;
		CString strInfoLine1;
		m_EditInfo.SetWindowText(_T(""));
		int iLength = 0;

		while( rCode != FTO && rCode != FERROR )
		{
			// Test if user pressed finish button
			if( true == m_bUserAborted )
			{
				if( IDYES == TASApp.AfxLocalizeMessageBox( IDS_PANELCBI_ABORTCOM, MB_YESNO ) )
				{
					m_iDTState=0;
					m_pclWizardCBI->Finish();
					return true;
				}
				else
				{
					m_bUserAborted = false;
				}
			}
			
			rCode = m_pclWizardCBI->GetAnswer( &buf );
			
			if( FTO == rCode || FERROR == rCode )
			{
				break;
			}

			if( 24 == row && buf.Buffer[0] != 0x0D )					// End of transmission ?
			{
				pLD->CheckLoggedData();
				iLength = 0;					// New Log
				row = 14;
			}

			// Interpreter
			switch( row )
			{
				case 14:					// Site Name
					strTempBuf = m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length );
					break;

				case 15:					// Ref Name
					{
						// Test if there is something into the CBI
						if( strTempBuf == _T("\r") && buf.Buffer[0] == 0x0D )		// NO DATA LOGGED
						{
							break;
						}

						// Log number
						iLog ++;

						// Create a new CDS_LoggedData in the temporary Tab.
						m_pTADS->CreateObject( IDPtr, CLASS( CDS_LoggedData ) );
						pTab->Insert( IDPtr );

						pLD = (CDS_LoggedData*)IDPtr.MP;
						pLD->SetSite((LPCTSTR)strTempBuf);
				
						pLD->SetRef(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length));
					}
					break;
		
				case 16:					// Valve n°
					{
						int valve;
						valve = _tcstol(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length),'\0',10);
						pLD->SetValveIndex((unsigned char)valve);				
					}
					break;
				
				case 17:					// Date -> CenturyYear.Month.Date
					pLD->SetDate(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length));	
					break;
				
				case 18:					// Hour -> Hour:Min:Sec
					{
						pLD->SetTime(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length));
						CString str,str1;
						str=TASApp.LoadLocalizedString(IDS_VALVE);
						str1.Format(_T("%d"),pLD->GetValveIndex());
						CTimeUnic dtu;
		 				dtu.CTime::CTime(pLD->GetLogDateTime());
					
						FormatString(strInfoLine1,TASApp.LoadLocalizedString(IDS_DATALOGGEDON),dtu.Format(IDS_DATE_FORMAT),dtu.Format(IDS_TIME_FORMAT));//(CString)pLD->GetDate(),(CString)pLD->GetTime());
						strInfoLine1 += (CString)_T("  [") + (CString)pLD->GetSite() + (CString)_T("; ") 
							+ (CString)pLD->GetRef() + (CString)_T("; ") + str + (CString)_T(" ") 
							+ str1 + (CString)_T("]");
						m_EditInfo.SetWindowText(strInfoLine1);
					
						TCHAR s[10];
						_stprintf_s(s,SIZEOFINTCHAR(s),_T("%d"),iLog);
						FormatString(str,IDS_PANELCBI3_LOG,s);
						pLD->SetName((LPCTSTR)str);
					}
					break;
				
				case 19:					// Interval
					{
						int interval;
						interval = _tcstol(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length),'\0',10);
						pLD->SetInterval(interval);
					}
					break;
				
				case 20:					// Total logged data
					iLength = _tcstol(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length),'\0',10);
					m_Progress.SetRange(0,100);//len);
					m_Progress.SetPos(0);
					break;	
				
				case 21:					// Mode : Pressure 0; flow 1; Temperature 2; pressure & temp 3; flow & temp 4
					{
						int mode;
						mode = _tcstol(m_pclWizardCBI->Byte2TChar(buf.Buffer,buf.Length),'\0',10);
						pLD->SetMode(mode);
					}
					break;
			}
		
			if( 22 == row )									// values start at row 22, loop on values
			{
				if( 0x0D == buf.Buffer[0] )
				{
					break;
				}

				CString strbuf = m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length );
				
				if( true == bDebug )
				{
					CString str;
					str.Format( _T("row[%d] : %s\n"), row, (LPCTSTR)strbuf );
					f.WriteTxtLine( (LPCTSTR)str );
				}

				int i = 1;
				double dVal;
				
				while( rCode != FTO && rCode != FERROR )
				{
					// Test if user pressed finish button.
					if( true == m_bUserAborted )
					{
						if( IDYES == TASApp.AfxLocalizeMessageBox( IDS_PANELCBI_ABORTCOM, MB_YESNO ) )
						{
							m_iDTState = 0;
							m_pclWizardCBI->Finish();
							return true;
						}
						else
						{
							m_bUserAborted = false;
						}
					}
					strbuf = m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length );
					
					if( true == bDebug )
					{
						CString str;
						str.Format( _T("%s\n"), (LPCTSTR) strbuf );
						f.WriteTxtLine( (LPCTSTR)str );
					}
					
					dVal = _tcstod( (LPCTSTR)strbuf, _T('\0') );
					int col = (i-1)%2;
					
					// Convert to SI
					UnitDesign_struct ud;
					
					switch( pLD->GetMode() )
					{
						case CLog::LOGTYPE_DPTEMP:
							
							if( col )	// Temperature, no conversion needed
							{
								if( dVal < -273.15 )
								{
									dVal = -273.15;
								}
								break;
							}
						
						case CLog::LOGTYPE_DP:
							ud = m_pUnitDB->GetUnit( _U_DIFFPRESS, 2 );			// Always receive bar
							dVal = GetConvOf( ud ) * ( dVal + GetOffsetOf( ud ) );
							col = 0;
							break;

						case CLog::LOGTYPE_FLOWTEMP:
							
							if( col )	// Temperature, no conversion needed
							{
								if( dVal < -273.15 )
								{
									dVal = -273.15;
								}

								break;
							}
						
						case CLog::LOGTYPE_FLOW:
							ud = m_pUnitDB->GetUnit( _U_FLOW, 1 );				// Always receive m³/h
							dVal = GetConvOf( ud ) * ( dVal + GetOffsetOf( ud ) );
							col = 0;
							break;

						case CLog::LOGTYPE_TEMP:				// Temperature:	no conversion needed !!!
							
							if( dVal < -273.15 )
							{
								dVal = -273.15;				// Always receive °C
							}
							
							col = 0;
							break;
					}
					
					pLD->AddValue( col, dVal );
					i++;
					
					if( iLength )
					{
						m_Progress.SetPos( i * 100 / iLength );
						CString str, str1, str2;
						str1.Format( _T("%d"), i );
						str2.Format( _T("%d"), iLength );
						FormatString( str, IDS_XLOGRECEIVED, str1, str2 );
						str = strInfoLine1 + (CString)_T("\r\n") + str; 
						m_EditInfo.SetWindowText( str );
					}

					rCode = m_pclWizardCBI->GetAnswer( &buf );
					
					if( FTO == rCode || FERROR == rCode || 0x0D == buf.Buffer[0] )
					{
						break;
					}
				}
				
				if( true == bDebug )
				{
					CString str;
					str.Format( _T("\n") );
					f.WriteTxtLine( (LPCTSTR)str );
				}

				row = 24;
			}
			else
			{
				if( 0x0D == buf.Buffer[0] )
				{
					break;
				}

				if( true == bDebug )
				{
					CString str;
					str.Format( _T("row[%d] : %s\n"), row, m_pclWizardCBI->Byte2TChar( buf.Buffer, buf.Length ) );
					f.WriteTxtLine( (LPCTSTR)str );
				}
				
				row++;
			}
		}
		
		m_EditInfo.SetWindowText( _T("") );
		
		if( true == bDebug ) 
		{
			f.Close();
		}
		
		if( FTO == rCode || FERROR == rCode )
		{
			return false;
		}
		
		return true;
	}
	catch( CHySelectException &clHySelectException )
	{
		clHySelectException.AddMessage( _T("Error in 'CPanelCBI3::CollectLoggedData'."), __LINE__, __FILE__ );
		throw;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPanelCBI3 message handlers

BOOL CPanelCBI3::OnInitDialog() 
{
	CDlgWizard::OnInitDialog();
	
	// Set the white rectangle to the first drawn window
	GetDlgItem(IDC_STATICRECTANGLE)->BringWindowToTop();
	GetDlgItem(IDC_STATICRECTANGLEINFO)->BringWindowToTop();
	m_brWhiteBrush.CreateSolidBrush(RGB(255,255,255));

	m_pclWizardCBI = (CWizardCBI *)GetWizMan();
	
	CString str;
	str=TASApp.LoadLocalizedString(IDS_PANELCBI3_TEXT);
	GetDlgItem(IDC_EDITTEXT)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI3_STATICCOMPROG);
	GetDlgItem(IDC_STATICCOMPROG)->SetWindowText(str);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI_SENDPROJ);
	GetDlgItem(IDC_STATICLINE1)->SetWindowText(str);
	GetDlgItem(IDC_STATICLINE1)->EnableWindow(false);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI_GETPLANT);
	GetDlgItem(IDC_STATICLINE2)->SetWindowText(str);
	GetDlgItem(IDC_STATICLINE2)->EnableWindow(false);
	str=TASApp.LoadLocalizedString(IDS_PANELCBI_COLLECTDATA);
	GetDlgItem(IDC_STATICLINE3)->SetWindowText(str);
	GetDlgItem(IDC_STATICLINE3)->EnableWindow(false);

	CFont fontbold;
	VERIFY(fontbold.CreateFont(
			-12,                        // nHeight
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_HEAVY,		 		   // nWeight
			FALSE,                     // bItalic
			FALSE,                     // bUnderline
			0,                         // cStrikeOut
			ANSI_CHARSET,              // nCharSet
			OUT_DEFAULT_PRECIS,        // nOutPrecision
			CLIP_DEFAULT_PRECIS,       // nClipPrecision
			DEFAULT_QUALITY,           // nQuality
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
			_T("Arial")));                 // lpszFacename
	((CStatic*)GetDlgItem(IDC_STATICCOMPROG))->SetFont(&fontbold);
	
	// LoadMappedBitmap
	m_BitmapArrow.LoadMappedBitmap(IDB_RARROW);
	HBITMAP hBitmap = (HBITMAP) m_BitmapArrow.GetSafeHandle();
	((CStatic*)GetDlgItem(IDC_STATICARROW1))->SetBitmap(hBitmap);
	((CStatic*)GetDlgItem(IDC_STATICARROW2))->SetBitmap(hBitmap);
	((CStatic*)GetDlgItem(IDC_STATICARROW3))->SetBitmap(hBitmap);

	GetDlgItem(IDC_STATICARROW1)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATICARROW2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATICARROW3)->ShowWindow(SW_HIDE);


	m_pTADS = TASApp.GetpTADS();
	m_pUnitDB = CDimValue::AccessUDB();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CPanelCBI3::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	
	if (pWnd->m_hWnd == GetDlgItem(IDC_EDITTEXT)->m_hWnd ||
		pWnd->m_hWnd == GetDlgItem(IDC_EDITINFO)->m_hWnd)
	{
		pDC->SetTextColor(RGB(0,0,0));
		pDC->SetBkColor(RGB(255,255,255));
		return (HBRUSH) m_brWhiteBrush;
	}
	
	hbr = CDlgWizard::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

LRESULT CPanelCBI3::OnDataTransferReady(WPARAM wParam, LPARAM lParam)
{
	m_pclWizardCBI->DisableButtons(CWizardManager::WizButNext);
	GetDlgItem(IDC_STATICARROW1)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATICARROW2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATICARROW3)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATICLINE1)->EnableWindow(false);
	GetDlgItem(IDC_STATICLINE2)->EnableWindow(false);
	GetDlgItem(IDC_STATICLINE3)->EnableWindow(false);
	bool flag=true;

	//====================
	// SITE BUILDER
	if (m_iDTState == 0)
	{
		m_pclWizardCBI->EnableButtons(CWizardManager::WizButFinish);
		if (m_pclWizardCBI->m_PanelCBI2.GetCheckSendProj())
		{
			GetDlgItem(IDC_STATICARROW1)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATICLINE1)->EnableWindow(true);

			// Do transmission
			int RetCode = SiteBuilder();
			if (RetCode == -1)		// ERROR MESSAGE
			{
				CString str; 
				FormatString( str, AFXMSG_PLANTALREADYEXIST, m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name ) );
				int nRet = ::AfxMessageBox( str, MB_YESNO | MB_ICONEXCLAMATION );
				
				if( IDNO == nRet )
				{
					flag = true;
				}
				else if( IDYES == nRet)
				{
					// Erase site
					BYTE Txbuf[256];
					CWizardCBI::s_Msg Rx;
					memcpy( Txbuf, ERASESITE, HEADERSIZE );
					CopyTextWithSpace( &Txbuf[HEADERSIZE], m_pTADS->GetpProjectRef()->GetString( CDS_ProjectRef::Name ), 21, true, 0 );

					m_pclWizardCBI->SendMessage(Txbuf);				
					// Wait Acknowledge from CBI
					if (m_pclWizardCBI->GetAnswer(&Rx) != FACK )
					{
						CString str;
						str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
						::AfxMessageBox((LPCTSTR)str,MB_ICONEXCLAMATION);
					}
					
					::PostMessage(m_hWnd,WM_USER_DATATRANSFERREADY,m_iDTState,0);
					return 0;
				}
			}
			else if (RetCode == -2)
			{
				CString str; str=TASApp.LoadLocalizedString(AFXMSG_INCOMPLETEPROJECT);
				AfxMessageBox(str,MB_ICONEXCLAMATION);
				m_pclWizardCBI->Finish();
				return 0;
			}
			else if (RetCode == 0)	// No Answer
			{
				CString str;
				str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
				if (AfxMessageBox((LPCTSTR)str,MB_RETRYCANCEL|MB_ICONEXCLAMATION)== IDRETRY)
					::PostMessage(m_hWnd,WM_USER_DATATRANSFERREADY,m_iDTState,0);
				else
					m_pclWizardCBI->Finish();
			}
		}
		if (flag) 
		{	
			GetDlgItem(IDC_STATICARROW1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATICLINE1)->EnableWindow(false);
			m_iDTState = 1;
		}
	}
	// Test if user pressed finish button
	if (m_bUserAborted)
	{
		if (TASApp.AfxLocalizeMessageBox(IDS_PANELCBI_ABORTCOM,MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
		{
			m_iDTState=0;
			m_pclWizardCBI->Finish();
		}
		else
			m_bUserAborted = false;
	}

	
	//====================
	// Collect SAVED DATA
	if (m_iDTState == 1)
	{
		m_pclWizardCBI->EnableButtons(CWizardManager::WizButFinish);
		if (m_pclWizardCBI->m_PanelCBI2.GetCheckReadProj())
		{
			GetDlgItem(IDC_STATICARROW2)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATICLINE2)->EnableWindow(true);
			// Keep a trace of modified flag before loading Logged Data into the temporary table
			char bModified = m_pTADS->IsModified();
			// Do transmission
			flag = CollectSavedData();
			// Reset flag Modified
			m_pTADS->Modified(bModified);
		}
		if (flag) 
		{	
			GetDlgItem(IDC_STATICARROW2)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATICLINE2)->EnableWindow(false);
			m_iDTState = 2;
		}
		else		// Transmission fails
		{
			DeleteTmpSavedData();
			CString str;
			str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
			if (AfxMessageBox((LPCTSTR)str,MB_RETRYCANCEL|MB_ICONEXCLAMATION)== IDRETRY)
				::PostMessage(m_hWnd,WM_USER_DATATRANSFERREADY,m_iDTState,0);
			else
				m_pclWizardCBI->Finish();
			flag=1;
		}
	}
	
	//====================
	// Collect LOGGED DATA
	if (m_iDTState == 2)
	{
		m_pclWizardCBI->EnableButtons(CWizardManager::WizButFinish);
		if (m_pclWizardCBI->m_PanelCBI2.GetCheckCollectData())
		{
			GetDlgItem(IDC_STATICARROW3)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATICLINE3)->EnableWindow(true);
			// Keep a trace of modified flag before loading Logged Data into the temporary table
			char bModified = m_pTADS->IsModified();
			flag=CollectLoggedData();
			// Reset flag Modified
			m_pTADS->Modified(bModified);
		}
		if (flag)		// Transmission OK 
		{	
			GetDlgItem(IDC_STATICLINE3)->EnableWindow(false);
			GetDlgItem(IDC_STATICARROW3)->ShowWindow(SW_HIDE);
			m_iDTState = 0;
		}
		else
		{
			// Delete all temporary CDS_LoggedData.
			CTable *pTab = (CTable *)m_pTADS->Get(_T("TMPLOGDATA_TAB")).MP; ASSERT(pTab);
		
			// Clear all CDS_LoggedData from the temporary tab.
			for( IDPTR IDPtr = pTab->GetFirst(); *IDPtr.ID; )
			{
				IDPTR IDPtrNext = pTab->GetNext();
				pTab->Remove( IDPtr );
				m_pTADS->DeleteObject( IDPtr );
				IDPtr = IDPtrNext;
			}

			CString str;
			str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
			if (AfxMessageBox((LPCTSTR)str,MB_RETRYCANCEL|MB_ICONEXCLAMATION)== IDRETRY)
				::PostMessage(m_hWnd,WM_USER_DATATRANSFERREADY,m_iDTState,0);
			else
				m_pclWizardCBI->Finish();
			flag = 1;
		}
	}
	if (!flag)
	{
		if (m_bUserAborted)				// User aborted transmission 
			return 0;

		CString str;
		str=TASApp.LoadLocalizedString(AFXMSG_NOCOM);
		if(AfxMessageBox((LPCTSTR)str,MB_RETRYCANCEL|MB_ICONQUESTION)==IDCANCEL)
		{
			m_iDTState=0;
			m_bUserAborted=true;		
			m_pclWizardCBI->Finish();
		}
		if (m_iDTState)
		{
			::PostMessage(m_hWnd,WM_USER_DATATRANSFERREADY,m_iDTState,0);
		}
	}
	else
	{
		CString str;
		str=TASApp.LoadLocalizedString(IDS_PANELCBI3_ALLOPDONE);
		m_EditInfo.SetWindowText(str);
		m_Progress.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATICCOMPROG)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATICLINE1)->EnableWindow(false);
		GetDlgItem(IDC_STATICLINE2)->EnableWindow(false);
		GetDlgItem(IDC_STATICLINE3)->EnableWindow(false);

		if (m_pclWizardCBI->m_PanelCBI2.GetCheckReadProj() || m_pclWizardCBI->m_PanelCBI2.GetCheckCollectData())
		{
			m_pclWizardCBI->EnableButtons(CWizardManager::WizButNext);
			m_pclWizardCBI->DisableButtons(CWizardManager::WizButFinish);
		}
		else	// Only send project was active  active Finish Button
		{
			m_pclWizardCBI->EnableButtons(CWizardManager::WizButFinish);
			m_pclWizardCBI->SetButtonText(CWizardManager::WizButFinish,IDS_WIZBUT_FINISH);
			m_pclWizardCBI->ShowButtons(CWizardManager::WizButNext,SW_HIDE);
			m_bUserAborted = true;		
		}

	}

	return 0;
}

BOOL CPanelCBI3::OnWizNext()
{
	::PostMessage(m_pclWizardCBI->m_hWnd,WM_USER_INITCOMPORT,0,0); 
	return true;
}

BOOL CPanelCBI3::OnWizFinish()
{
	if( true == m_bUserAborted )
	{
		::PostMessage( m_pclWizardCBI->m_hWnd, WM_USER_INITCOMPORT, 0, 0 ); 
		return true;
	}
	m_bUserAborted = true;
	return false;
}

bool CPanelCBI3::OnActivate()
{
	m_iDTState=0;
		
	m_pclWizardCBI->DisableButtons(CWizardManager::WizButNext);
	m_bUserAborted = false;

	::PostMessage(m_hWnd,WM_USER_DATATRANSFERREADY,m_iDTState,0);
	return true;
}

bool CPanelCBI3::OnAfterActivate()
{
	// Just to have the focus on something and to be able to intercept the [ESCAPE] in the 'CDlgWizard' base class.
	SetFocus();
	return true;
}
