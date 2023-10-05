#pragma once

#include "HySelectException.h"

// See 'TAS Project\TASelect-Sln\Doc\Code\Exceptions - Overview of the standard exception handling techniques provided by Visual C++.docx'
// to have explanations.

class CHySelectExceptionHandler
{
public:
	CHySelectExceptionHandler( CWinAppEx *pParent );
	~CHySelectExceptionHandler();

	// Sets exception handlers that work on per-process basis
    void SetProcessExceptionHandlers();

    // Installs C++ exception handlers that function on per-thread basis
    void SetThreadExceptionHandlers();

	// Exception handler functions.

	static LONG WINAPI SehHandler( PEXCEPTION_POINTERS pExceptionPtrs );

	// This method is called for the managed 'CHySelectException'.
	void HyselectExceptionHandler( CHySelectException &clHySelectException );

	// When CRT encounters an unhandled C++ typed exception, it calls the terminate() function.
	// To intercept such calls and take an appropriate action, you should set the error handler using the set_terminate() function.
	static void TerminateHandler();

	static void UnexpectedHandler();


	// Use the _set_purecall_handler() function to handle pure virtual function calls. This function can be used in VC++ .NET 2003 and later.
	// This function works for all threads of the caller process.
	static void PureCallHandler();

	// Use the _set_new_handler() function to handle memory allocation faults. This function can be used in VC++ .NET 2003 and later.
	// This function works for all threads of the caller process. Consider using the _set_new_mode() function to define error 
	// behaviour for the malloc() function.

	// Comment for "int _set_new_mode( int newhandlermode )".
	// The C++ _set_new_mode function sets the new handler mode for malloc. The new handler mode indicates whether, on failure, malloc is 
	// to call the new handler routine as set by _set_new_handler. By default, malloc does not call the new handler routine on failure to 
	// allocate memory. You can override this default behavior so that, when malloc fails to allocate memory, malloc calls the new handler
	// routine in the same way that the new operator does when it fails for the same reason.

	static int NewHandler( size_t iSizeToAllocate );

	// Use the _set_invalid_parameter_handler() function to handle situations when CRT detects an invalid argument in a system function call. 
	// This function can be used in VC++ 2005 and later. This function works for all threads of the caller process.
	static void InvalidParameterHandler( const TCHAR *ptcExpression, const TCHAR *ptcFunction, const TCHAR *ptcFile, unsigned int uiLine, uintptr_t pReserved );

	// The SIGABRT signal is sent to a process to tell it to abort, i.e. to terminate. The signal is usually initiated by the process itself 
	// when it calls abort() function of the C Standard Library, but it can be sent to the process from outside like any other signal.
	static void SigabrtHandler( int );

	// The SIGFPE signal reports a fatal arithmetic error. Although the name is derived from “floating-point exception”, this signal actually 
	// covers all arithmetic errors, including division by zero and overflow. If a program stores integer data in a location which is then used
	// in a floating-point operation, this often causes an “invalid operation” exception, because the processor cannot recognize the data as a floating-point number.
	// iCode can have these different values: 
	//		_FPE_INVALID (0x81)	   -> Invalid result
	//		_FPE_DENORMAL (0x82)   -> Denormal operand
	//		_FPE_ZERODIVIDE (0x83) -> Divide by zero
	//		_FPE_OVERFLOW (0x84)   -> Integer overflow (impossible in a C program unless you enable overflow trapping in a hardware-specific fashion).
	//		_FPE_UNDERFLOW (0x85)  -> Underflow
	//		_FPE_INEXACT (0x86)    -> Inexact precision
	static void SigfpeHandler( int iCode, int iSubCode );

	// The SIGINT signal is sent to a process by its controlling terminal when a user wishes to interrupt the process. This is typically initiated
	// by pressing Ctrl+C, but on some systems, the "delete" character or "break" key can be used.[9]
	static void SigintHandler( int );

	// The name of this signal is derived from “illegal instruction”; it usually means your program is trying to execute garbage or a
	// privileged instruction. Since the C compiler generates only valid instructions, SIGILL typically indicates that the executable file 
	// is corrupted, or that you are trying to execute data. Some common ways of getting into the latter situation are by passing an invalid
	// object where a pointer to a function was expected, or by writing past the end of an automatic array (or similar problems with pointers to
	// automatic variables) and corrupting other data on the stack such as the return address of a stack frame.
	// SIGILL can also be generated when the stack overflows, or when the system has trouble running the handler for a signal.
	static void SigillHandler( int );

	// This signal is generated when a program tries to read or write outside the memory that is allocated for it, or to write memory that
	// can only be read. (Actually, the signals only occur when the program goes far enough outside to be detected by the system’s memoryµ
	// protection mechanism.) The name is an abbreviation for “segmentation violation”.
	// Common ways of getting a SIGSEGV condition include dereferencing a null or uninitialized pointer, or when you use a pointer to step
	// through an array, but fail to check for the end of the array. It varies among systems whether dereferencing a null pointer generates
	// SIGSEGV or SIGBUS.
	static void SigsegvHandler( int );

	// The SIGTERM signal is sent to a process to request its termination. Unlike the SIGKILL signal, it can be caught and interpreted or 
	// ignored by the process. This allows the process to perform nice termination releasing resources and saving state if appropriate.
	// SIGINT is nearly identical to SIGTERM.
	static void SigtermHandler( int );

// Public variables.
public:
	static bool m_bCurrentProjectSaved;
	static bool m_bMiniDumpCreated;
	static EXCEPTION_POINTERS *m_pExceptionPtrs;
	static CString m_strTimestamp;

	// Because during the execepption handling we can have one other excpetion, we need to manage with a state engine.
	// Remark: 'HEH' is for Hyselect Exception Handler.
	enum
	{
		HSH_NotRunning,
		HSH_SaveCurrentProject,
		HSH_GetExceptionHandler,
		HSH_CreateMiniDump,
		HSH_ShowExceptionMessage,
		HSH_Terminate,
	};
	static int m_iStateEngine;

// Private methods.
private:
	// Allow to verify the execution of the exception handling.
	// Remark: needed in case of re-entrance (1 crash in HySelect and one when managing exceptions).
	static void _VerifyReEntrance();

	// Run the execution of the exception handling.
	static void _RunStateEngine();

	// Try to save the current project opened.
	static bool _SaveCurrentProject();

	// Collects current process state.
    static bool _GetExceptionPointers( DWORD dwExceptionCode );

	// This method creates minidump of the process
	static bool _CreateMiniDump();

	static void _ShowExceptionDialogMessage( bool bMiniDumpCreated, bool bCurrentProjectSaved );

// Private variables.
private:
	CWinAppEx *m_pParent;
};
