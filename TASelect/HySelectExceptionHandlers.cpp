#include "stdafx.h"


#include <signal.h>
#include <dbghelp.h>
#include <intrin.h>

#include "HySelectExceptionHandlers.h"
#include "DlgExceptionMessage.h"
#include "TASelect.h"


#ifndef _AddressOfReturnAddress

// Taken from: http://msdn.microsoft.com/en-us/library/s975zw7k(VS.71).aspx
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

// _ReturnAddress and _AddressOfReturnAddress should be prototyped before use 
EXTERNC void * _AddressOfReturnAddress(void);
EXTERNC void * _ReturnAddress(void);

#endif 

bool CHySelectExceptionHandler::m_bCurrentProjectSaved =  false;
bool CHySelectExceptionHandler::m_bMiniDumpCreated = false;
EXCEPTION_POINTERS *CHySelectExceptionHandler::m_pExceptionPtrs = NULL;
CString CHySelectExceptionHandler::m_strTimestamp = _T("");
int CHySelectExceptionHandler::m_iStateEngine = CHySelectExceptionHandler::HSH_NotRunning;

CHySelectExceptionHandler::CHySelectExceptionHandler( CWinAppEx *pParent )
{
	m_pParent = pParent;
}

CHySelectExceptionHandler::~CHySelectExceptionHandler()
{
}

void CHySelectExceptionHandler::SetProcessExceptionHandlers()
{
	// Install top-level SEH handler.
	SetUnhandledExceptionFilter( SehHandler );

	// Catch pure virtual function calls.
	// Because there is one _purecall_handler for the whole process, 
	// calling this function immediately impacts all threads. The last 
	// caller on any thread sets the handler. 
	// http://msdn.microsoft.com/en-us/library/t296ys27.aspx
	_set_purecall_handler( PureCallHandler );

	// Catch new operator memory allocation exceptions.
	_set_new_handler( NewHandler );

	// Catch invalid parameter exceptions.
	_set_invalid_parameter_handler( InvalidParameterHandler ); 

	// Set up C++ signal handlers

	_set_abort_behavior( _CALL_REPORTFAULT, _CALL_REPORTFAULT );

	// Catch an abnormal program termination.
	signal( SIGABRT, SigabrtHandler );

	// Catch illegal instruction handler
	signal( SIGINT, SigintHandler );

	// Catch a termination request
	signal( SIGTERM, SigtermHandler );
}

void CHySelectExceptionHandler::SetThreadExceptionHandlers()
{
	// Catch terminate() calls. 
	// In a multithreaded environment, terminate functions are maintained 
	// separately for each thread. Each new thread needs to install its own 
	// terminate function. Thus, each thread is in charge of its own termination handling.
	// http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
	set_terminate( TerminateHandler );

	// Catch unexpected() calls.
	// In a multithreaded environment, unexpected functions are maintained 
	// separately for each thread. Each new thread needs to install its own 
	// unexpected function. Thus, each thread is in charge of its own unexpected handling.
	// http://msdn.microsoft.com/en-us/library/h46t5b69.aspx  
	set_unexpected( UnexpectedHandler );

	// Catch a floating point error.
	typedef void (*sigh)(int);
	signal( SIGFPE, (sigh)SigfpeHandler );

	// Catch an illegal instruction.
	signal( SIGILL, SigillHandler );

	// Catch illegal storage access errors.
	signal( SIGSEGV, SigsegvHandler );
}

LONG WINAPI CHySelectExceptionHandler::SehHandler( PEXCEPTION_POINTERS pExceptionPtrs )
{
	// Structured exception handler.

	_VerifyReEntrance();
	m_pExceptionPtrs = pExceptionPtrs;
	_RunStateEngine();

	// Unreacheable code.
	return EXCEPTION_EXECUTE_HANDLER;
}

void CHySelectExceptionHandler::HyselectExceptionHandler( CHySelectException &clHySelectException )
{
	// Exception internaly managed with a 'throw CHySelectException( , __LINE__, __FILE__ )' class.

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT terminate() call handler.
void __cdecl CHySelectExceptionHandler::TerminateHandler()
{
	// Abnormal program termination (terminate() function was called).

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT unexpected() call handler
void __cdecl CHySelectExceptionHandler::UnexpectedHandler()
{
	// Unexpected error (unexpected() function was called).

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT Pure virtual method call handler.
void __cdecl CHySelectExceptionHandler::PureCallHandler()
{
	// Pure virtual function call.

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT new operator fault handler.
int __cdecl CHySelectExceptionHandler::NewHandler( size_t iSizeToAllocate )
{
	// 'new' operator memory allocation exception.

	_VerifyReEntrance();
	_RunStateEngine();
	
	// Unreacheable code.
	return 0;
}

// CRT invalid parameter handler.
void __cdecl CHySelectExceptionHandler::InvalidParameterHandler( const TCHAR *ptcExpression, const TCHAR *ptcFunction, const TCHAR *ptcFile, unsigned int uiLine, uintptr_t pReserved )
{
	// Invalid parameter exception.

	_VerifyReEntrance();
	_RunStateEngine();

	// Terminate process.
	TerminateProcess( GetCurrentProcess(), 1 );
}

// CRT SIGABRT signal handler.
void CHySelectExceptionHandler::SigabrtHandler( int )
{
	// Caught SIGABRT C++ signal.

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT SIGFPE signal handler.
void CHySelectExceptionHandler::SigfpeHandler( int iCode, int iSubCode )
{
	// Floating point exception (SIGFPE).

	_VerifyReEntrance();
	m_pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
	_RunStateEngine();
}

// CRT sigint signal handler.
void CHySelectExceptionHandler::SigintHandler( int )
{
	// Interruption (SIGINT).

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT sigill signal handler.
void CHySelectExceptionHandler::SigillHandler( int )
{
	// Illegal instruction (SIGILL).

	_VerifyReEntrance();
	_RunStateEngine();
}

// CRT SIGSEGV signal handler.
void CHySelectExceptionHandler::SigsegvHandler( int )
{
	// Invalid storage access (SIGSEGV).

	_VerifyReEntrance();
	m_pExceptionPtrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
	_RunStateEngine();
}

// CRT SIGTERM signal handler.
void CHySelectExceptionHandler::SigtermHandler( int )
{
	// Termination request (SIGTERM).

	_VerifyReEntrance();
	_RunStateEngine();
}

void CHySelectExceptionHandler::_VerifyReEntrance()
{
	switch( m_iStateEngine )
	{
		case HSH_NotRunning:
			// OK: first time we catch exception, we start by saving the projet.
			m_iStateEngine = HSH_SaveCurrentProject;
			break;

		case HSH_SaveCurrentProject:
			// Oups: exception handler is already running. It seems that there was a problem when 
			// trying to save the current project. We bypass this step and try the next one.
			m_bCurrentProjectSaved = false;
			m_iStateEngine = HSH_GetExceptionHandler;
			break;

		case HSH_GetExceptionHandler:
			// Oups: exception handler is already running. It seems that there was a problem when 
			// trying to get exception handler. We can't create the mini dump file, we directly go
			// to show exception message.
			m_bMiniDumpCreated = false;
			m_iStateEngine = HSH_ShowExceptionMessage;
			break;

		case HSH_CreateMiniDump:
			// Oups: exception handler is already running. It seems that there was a problem when 
			// trying to create the mini dump file. We bypass this step and try the next one.
			m_bMiniDumpCreated = false;
			m_iStateEngine = HSH_ShowExceptionMessage;
			break;

		case HSH_ShowExceptionMessage:
			// Oups: exception handler is already running. It seems that there was a problem when 
			// showing the exception message dialog. We bypass this step and go the terminate.
			m_iStateEngine = HSH_Terminate;
			break;
	}
}

void CHySelectExceptionHandler::_RunStateEngine()
{
	do
	{
		switch( m_iStateEngine )
		{
			case HSH_SaveCurrentProject:
				// Try now to save the current opened project.
				m_bCurrentProjectSaved = _SaveCurrentProject();
				m_iStateEngine = HSH_GetExceptionHandler;
				break;

			case HSH_GetExceptionHandler:

				if( NULL == m_pExceptionPtrs )
				{
					m_bMiniDumpCreated = false;

					if( true == _GetExceptionPointers( 0 ) )
					{
						// We succeed to retrieve the pointer, we can go the the creation of the mini dump file.
						m_iStateEngine = HSH_CreateMiniDump;
					}
					else
					{
						// Can't retrieve the pointer, we can't create the mini dump file. We directly go to 
						// show exeception message.
						m_iStateEngine = HSH_ShowExceptionMessage;
					}
				}
				else
				{
					// We already have the pointer (Depends of the exception handler), we can directly go the
					// creation of the mini dump file.
					m_iStateEngine = HSH_CreateMiniDump;
				}

				break;

			case HSH_CreateMiniDump:
				// Write minidump file.
				m_bMiniDumpCreated = _CreateMiniDump();
				m_iStateEngine = HSH_ShowExceptionMessage;
				break;

			case HSH_ShowExceptionMessage:
				_ShowExceptionDialogMessage( m_bCurrentProjectSaved, m_bMiniDumpCreated );
				m_iStateEngine = HSH_Terminate;
				break;

			case HSH_Terminate:
				// Terminate process
				TerminateProcess( GetCurrentProcess(), 1 );
				// Nothing will happen after this method.
				break;
		}
	
	}while( 1 );

	// Remark: We can set 'while(1)' because once 'HSH_Terminate' is executed, HySelect is killed.
}

bool CHySelectExceptionHandler::_SaveCurrentProject()
{
	try
	{
		CTADatastruct *pTADS = TASApp.GetpTADS();

		if( NULL == pTADS )
		{
			return false;
		}
		
		pTADS->CleanClipboard();

		CString strPath = TASApp.GetLogsPath();

		// Check if folder exist.
		if( FALSE == PathFileExists( strPath ) )
		{
			if( ERROR_SUCCESS != SHCreateDirectoryEx( 0, ( LPCTSTR )strPath, NULL ) )
			{
				return false;
			}
		}

		CTime clTime = CTime::GetCurrentTime();
		m_strTimestamp.Format( _T("%i-%02i-%02i-%02i-%02i-%02i"), clTime.GetYear(), clTime.GetMonth(), clTime.GetDay(), clTime.GetHour(), clTime.GetMinute(), clTime.GetSecond() );
		strPath.Format( _T("%s%s-project.tsp"), strPath, m_strTimestamp );
		
		// Update datastruct with current user pipes.
		TASApp.GetpPipeDB()->CopyTo( pTADS, true );

		pTADS->SetVersion( CTADATASTRUCT_FORMAT_VERSION );
		std::ofstream outf( strPath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc );
		pTADS->Write( outf );
		outf.close();
		
		return true;
	}
	catch(...)
	{
		// We must here catch all exceptions.
		return false;
	}
}

void CHySelectExceptionHandler::_ShowExceptionDialogMessage( bool bCurrentProjectSaved, bool bMiniDumpCreated )
{
	CDlgExceptionMessage Dlg( bCurrentProjectSaved, bMiniDumpCreated, m_strTimestamp );
	Dlg.DoModal();
}

// The following code gets exception pointers using a workaround found in CRT code.
bool CHySelectExceptionHandler::_GetExceptionPointers( DWORD dwExceptionCode )
{
	try
	{
		// The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

		EXCEPTION_RECORD ExceptionRecord;
		CONTEXT ContextRecord;
		memset( &ContextRecord, 0, sizeof( CONTEXT ) );

#ifdef _X86_

		__asm {
			mov dword ptr [ContextRecord.Eax], eax
				mov dword ptr [ContextRecord.Ecx], ecx
				mov dword ptr [ContextRecord.Edx], edx
				mov dword ptr [ContextRecord.Ebx], ebx
				mov dword ptr [ContextRecord.Esi], esi
				mov dword ptr [ContextRecord.Edi], edi
				mov word ptr [ContextRecord.SegSs], ss
				mov word ptr [ContextRecord.SegCs], cs
				mov word ptr [ContextRecord.SegDs], ds
				mov word ptr [ContextRecord.SegEs], es
				mov word ptr [ContextRecord.SegFs], fs
				mov word ptr [ContextRecord.SegGs], gs
				pushfd
				pop [ContextRecord.EFlags]
		}

		ContextRecord.ContextFlags = CONTEXT_CONTROL;

#pragma warning(push)
#pragma warning(disable:4311)
		ContextRecord.Eip = (ULONG)_ReturnAddress();
		ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();

#pragma warning(pop)
		ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);

#elif defined (_IA64_) || defined (_AMD64_)

		/* Need to fill up the Context in IA64 and AMD64. */
		RtlCaptureContext( &ContextRecord );

#else  /* defined (_IA64_) || defined (_AMD64_) */

		ZeroMemory( &ContextRecord, sizeof( ContextRecord ) );

#endif  /* defined (_IA64_) || defined (_AMD64_) */

		ZeroMemory( &ExceptionRecord, sizeof( EXCEPTION_RECORD ) );

		ExceptionRecord.ExceptionCode = dwExceptionCode;
		ExceptionRecord.ExceptionAddress = _ReturnAddress();

		EXCEPTION_RECORD *pExceptionRecord = new EXCEPTION_RECORD;

		if( NULL == pExceptionRecord )
		{
			return false;
		}
		
		memcpy( pExceptionRecord, &ExceptionRecord, sizeof( EXCEPTION_RECORD ) );
		
		CONTEXT *pContextRecord = new CONTEXT;

		if( NULL == pContextRecord )
		{
			return false;
		}
		
		memcpy( pContextRecord, &ContextRecord, sizeof( CONTEXT ) );

		m_pExceptionPtrs = new EXCEPTION_POINTERS;

		if( NULL == m_pExceptionPtrs )
		{
			return false;
		}
		
		m_pExceptionPtrs->ExceptionRecord = pExceptionRecord;
		m_pExceptionPtrs->ContextRecord = pContextRecord;

		return true;
	}
	catch(...)
	{
		// We must here catch all exceptions.
		return false;
	}
}

bool CHySelectExceptionHandler::_CreateMiniDump()
{
	try
	{
		HMODULE hDbgHelp = NULL;
		HANDLE hFile = NULL;
		MINIDUMP_EXCEPTION_INFORMATION mei;
		MINIDUMP_CALLBACK_INFORMATION mci;
    
		// Load dbghelp.dll.
		hDbgHelp = LoadLibrary( _T("dbghelp.dll") );
    
		if( NULL == hDbgHelp )
		{
			// Error - couldn't load dbghelp.dll.
			return false;
		}

		// Create the minidump file.
		CString strPath = TASApp.GetLogsPath();

		// Check if folder exist.
		if( FALSE == PathFileExists( strPath ) )
		{
			if( ERROR_SUCCESS != SHCreateDirectoryEx( 0, ( LPCTSTR )strPath, NULL ) )
			{
				return false;
			}
		}

		if( true == m_strTimestamp.IsEmpty() )
		{
			CTime clTime = CTime::GetCurrentTime();
			m_strTimestamp.Format( _T("%i-%02i-%02i-%02i-%02i-%02i"), clTime.GetYear(), clTime.GetMonth(), clTime.GetDay(), clTime.GetHour(), clTime.GetMinute(), clTime.GetSecond() );
		}

		strPath.Format( _T("%s%s-crashdump.dmp"), strPath, m_strTimestamp );

		hFile = CreateFile( strPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

		if( INVALID_HANDLE_VALUE == hFile )
		{
			// Couldn't create file.
			return false;
		}
   
		// Write minidump to the file.
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = m_pExceptionPtrs;
		mei.ClientPointers = FALSE;
		mci.CallbackRoutine = NULL;
		mci.CallbackParam = NULL;

		typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(
			HANDLE hProcess, 
			DWORD ProcessId, 
			HANDLE hFile, 
			MINIDUMP_TYPE DumpType, 
			CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
			CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam, 
			CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

		LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = (LPMINIDUMPWRITEDUMP)GetProcAddress( hDbgHelp, "MiniDumpWriteDump" );
    
		if( NULL == pfnMiniDumpWriteDump )
		{    
			// Bad MiniDumpWriteDump function
			return false;
		}

		HANDLE hProcess = GetCurrentProcess();
		DWORD dwProcessId = GetCurrentProcessId();

		BOOL bWriteDump = pfnMiniDumpWriteDump( hProcess, dwProcessId, hFile, MiniDumpNormal, &mei, NULL, &mci );

		if( FALSE == bWriteDump )
		{    
			// Error writing dump.
			return false;
		}

		// Close file.
		CloseHandle( hFile );

		// Unload dbghelp.dll.
		FreeLibrary( hDbgHelp );

		return true;
	}
	catch(...)
	{
		// We must here catch all exceptions.
		return false;
	}
}
