// STTubeDevice.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "usb100.h"
#include "STTubeDevice.h"
#include "STTubeDeviceErr30.h"
#include "STTubeDeviceTyp30.h"
#include "STDevicesMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/*
/////////////////////////////////////////////////////////////////////////////
// CSTTubeDeviceApp

BEGIN_MESSAGE_MAP(CSTTubeDeviceApp, CWinApp)
	//{{AFX_MSG_MAP(CSTTubeDeviceApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSTTubeDeviceApp construction

CSTTubeDeviceApp::CSTTubeDeviceApp()
{
	m_pMgr=NULL;
}
 
/////////////////////////////////////////////////////////////////////////////
// The one and only CSTTubeDeviceApp object

CSTTubeDeviceApp theApp;

BOOL CSTTubeDeviceApp::InitInstance() 
{
	m_pMgr=new CSTDevicesManager();
	return CWinApp::InitInstance();
}

int CSTTubeDeviceApp::ExitInstance() 
{
	if (m_pMgr)
		delete m_pMgr;
	m_pMgr=NULL;

	return CWinApp::ExitInstance();
}
*/

CSTDevicesManager *g_STTubeDevice_pSTDevicesManager = NULL;

/////////////////////////////////////////////////////////////////////////////
// Exported functions bodies

void STDevice_Init()
{
	if( NULL == g_STTubeDevice_pSTDevicesManager )
	{
		g_STTubeDevice_pSTDevicesManager = new CSTDevicesManager();
	}
}

void STDevice_DeInit()
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
	{
		delete g_STTubeDevice_pSTDevicesManager;
		g_STTubeDevice_pSTDevicesManager = NULL;
	}
}

DWORD STDevice_Open( LPSTR szDevicePath, 
							LPHANDLE phDevice, 
							LPHANDLE phUnPlugEvent)
{
	CStringA sSymbName;

	if (!sSymbName)
		return STDEVICE_BADPARAMETER;
	
	sSymbName=szDevicePath;
	if (sSymbName.IsEmpty())
		return STDEVICE_BADPARAMETER;

	if( NULL != g_STTubeDevice_pSTDevicesManager )
	{
		return g_STTubeDevice_pSTDevicesManager->Open(sSymbName, phDevice, phUnPlugEvent);
	}

	return STDEVICE_MEMORY;
}

DWORD STDevice_Close(HANDLE hDevice)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
	{
		return g_STTubeDevice_pSTDevicesManager->Close(hDevice);
	}

	return STDEVICE_MEMORY;
}

DWORD STDevice_OpenPipes(HANDLE hDevice)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
	{
		return g_STTubeDevice_pSTDevicesManager->OpenPipes(hDevice);
	}

	return STDEVICE_MEMORY;
}

DWORD STDevice_ClosePipes(HANDLE hDevice)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
	{
		return g_STTubeDevice_pSTDevicesManager->ClosePipes(hDevice);
	}

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetStringDescriptor(HANDLE hDevice, 
								   UINT nIndex, 
								   LPSTR szString, 
								   UINT nStringLength)
{
	CStringA sString;
	DWORD nRet=STDEVICE_MEMORY;

	if( NULL != g_STTubeDevice_pSTDevicesManager )
	{
		// Check parameters we use here. Others are checked in the manager class
		if (!szString)
			return STDEVICE_BADPARAMETER;

		nRet=g_STTubeDevice_pSTDevicesManager->GetStringDescriptor(hDevice, nIndex, sString);

		if (nRet==STDEVICE_NOERROR)
			strncpy(szString, (LPCSTR)sString, nStringLength);
	}
	return nRet;
}

DWORD STDevice_GetDeviceDescriptor(HANDLE hDevice, 
								   PUSB_DEVICE_DESCRIPTOR pDesc)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetDeviceDescriptor(hDevice, pDesc);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetNbOfConfigurations(HANDLE hDevice, PUINT pNbOfConfigs)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetNbOfConfigurations(hDevice, pNbOfConfigs);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetConfigurationDescriptor(HANDLE hDevice, 
										  UINT nConfigIdx, 
										  PUSB_CONFIGURATION_DESCRIPTOR pDesc)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetConfigurationDescriptor(hDevice, nConfigIdx, pDesc);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetNbOfInterfaces(HANDLE hDevice, 
								 UINT nConfigIdx, 
								 PUINT pNbOfInterfaces)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetNbOfInterfaces(hDevice, nConfigIdx, pNbOfInterfaces);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetNbOfAlternates(HANDLE hDevice, 
								 UINT nConfigIdx, 
								 UINT nInterfaceIdx, 
								 PUINT pNbOfAltSets)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetNbOfAlternates(hDevice, nConfigIdx, nInterfaceIdx, pNbOfAltSets);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetInterfaceDescriptor(HANDLE hDevice, 
									  UINT nConfigIdx, 
									  UINT nInterfaceIdx, 
									  UINT nAltSetIdx, 
									  PUSB_INTERFACE_DESCRIPTOR pDesc)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetInterfaceDescriptor(hDevice, nConfigIdx, nInterfaceIdx, nAltSetIdx, pDesc);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetNbOfEndPoints(HANDLE hDevice, 
								UINT nConfigIdx, 
								UINT nInterfaceIdx, 
								UINT nAltSetIdx, 
								PUINT pNbOfEndPoints)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetNbOfEndPoints(hDevice, nConfigIdx, nInterfaceIdx, nAltSetIdx, pNbOfEndPoints);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetEndPointDescriptor(HANDLE hDevice, 
									 UINT nConfigIdx, 
									 UINT nInterfaceIdx, 
									 UINT nAltSetIdx, 
									 UINT nEndPointIdx, 
									 PUSB_ENDPOINT_DESCRIPTOR pDesc)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetEndPointDescriptor(hDevice, nConfigIdx, nInterfaceIdx, nAltSetIdx, nEndPointIdx, pDesc);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetNbOfDescriptors(HANDLE hDevice, 
										BYTE nLevel,
										BYTE nType,
										UINT nConfigIdx, 
										UINT nInterfaceIdx, 
										UINT nAltSetIdx, 
										UINT nEndPointIdx, 
										PUINT pNbOfDescriptors)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetNbOfDescriptors(hDevice, nLevel,
												   nType,
												   nConfigIdx, nInterfaceIdx, nAltSetIdx, nEndPointIdx, 
												   pNbOfDescriptors);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetDescriptor(HANDLE hDevice, 
								    BYTE nLevel,
									BYTE nType,
									UINT nConfigIdx, 
									UINT nInterfaceIdx, 
									UINT nAltSetIdx, 
									UINT nEndPointIdx, 
									UINT nIdx,
									PBYTE pDesc,
									UINT nDescSize)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetDescriptor(hDevice, nLevel,
											  nType, 
											  nConfigIdx, nInterfaceIdx, nAltSetIdx, nEndPointIdx, nIdx,
											  pDesc, nDescSize);

	return STDEVICE_MEMORY;
}

DWORD STDevice_SelectCurrentConfiguration(HANDLE hDevice, 
										  UINT nConfigIdx, 
										  UINT nInterfaceIdx, 
										  UINT nAltSetIdx)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->SelectCurrentConfiguration(hDevice, nConfigIdx, nInterfaceIdx, nAltSetIdx);

	return STDEVICE_MEMORY;
}

DWORD STDevice_SetDefaultTimeOut(HANDLE hDevice, DWORD nTimeOut)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->SetDefaultTimeOut(hDevice, nTimeOut);

	return STDEVICE_MEMORY;
}

DWORD STDevice_SetMaxNumInterruptInputBuffer(HANDLE hDevice,
													WORD nMaxNumInputBuffer)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->SetMaxNumInterruptInputBuffer(hDevice, nMaxNumInputBuffer);

	return STDEVICE_MEMORY;
}

DWORD STDevice_GetMaxNumInterruptInputBuffer(HANDLE hDevice,
													PWORD pMaxNumInputBuffer)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->GetMaxNumInterruptInputBuffer(hDevice, pMaxNumInputBuffer);

	return STDEVICE_MEMORY;
}

DWORD STDevice_SetSuspendModeBehaviour(HANDLE hDevice, BOOL Allow)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->SetSuspendModeBehaviour(hDevice, Allow);

	return STDEVICE_MEMORY;
}

DWORD STDevice_EndPointControl(HANDLE hDevice, 
							   UINT nEndPointIdx, 
							   UINT nOperation)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->EndPointControl(hDevice, nEndPointIdx, nOperation);

	return STDEVICE_MEMORY;
}

DWORD STDevice_Reset(HANDLE hDevice)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->Reset(hDevice);

	return STDEVICE_MEMORY;
}

DWORD STDevice_ControlPipeRequest(HANDLE hDevice, PCNTRPIPE_RQ pRequest,
										 PBYTE pData)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->ControlPipeRequest(hDevice, pRequest, pData);

	return STDEVICE_MEMORY;
}

DWORD STDevice_Read(HANDLE hDevice, 
				    UINT nEndPointIdx,
					PBYTE pBuffer, 
					PUINT pSize, 
					DWORD nTimeOut)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->Read(hDevice, nEndPointIdx, pBuffer, pSize, nTimeOut);

	return STDEVICE_MEMORY;
}

DWORD STDevice_Write(HANDLE hDevice, 
				    UINT nEndPointIdx,
					 PBYTE pBuffer, 
					 PUINT pSize, 
					 DWORD nTimeOut)
{
	if( NULL != g_STTubeDevice_pSTDevicesManager )
		return g_STTubeDevice_pSTDevicesManager->Write(hDevice, nEndPointIdx, pBuffer, pSize, nTimeOut);

	return STDEVICE_MEMORY;
}
