/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

// File: CrashHandler.cpp
// Description: Exception handling and report generation functionality.
// Authors: mikecarruth, zexspectrum
// Date: 



#include "CrashHandler.h"
#include <signal.h>
#include <time.h>
#include "Report.h"
#include "Dbghlp.h"

#pragma warning(disable: 4996)


// Get crash handlers of current process
CurrentProcessCrashHandler* GetCurrentProcessCrashHandler()
{
    static CurrentProcessCrashHandler instance;
    return &instance;
}


#define LOCK_HANDLER()      GetCurrentProcessCrashHandler()->critsec.Enter()
#define UNLOCK_HANDLER()    GetCurrentProcessCrashHandler()->critsec.Leave()

//////////////////////////////////////////////////////////////////////////
//
// Exception handler functions. 
//


static DWORD WINAPI StackOverflowThreadFunction(LPVOID lpParameter)
{
    PEXCEPTION_POINTERS pExceptionPtrs = reinterpret_cast<PEXCEPTION_POINTERS>(lpParameter);

    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Generate error report.
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_SEH_EXCEPTION;
    ei.pexcptrs = pExceptionPtrs;
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();

    return 0;
}

// Structured exception handler (SEH handler)
static LONG WINAPI SehHandler(__in PEXCEPTION_POINTERS pExceptionPtrs)
{
    // Handle stack overflow in a separate thread.
    // Vojtech: Based on martin.bis...@gmail.com comment in
    // http://groups.google.com/group/crashrpt/browse_thread/thread/a1dbcc56acb58b27/fbd0151dd8e26daf?lnk=gst&q=stack+overflow#fbd0151dd8e26daf
    if (pExceptionPtrs != 0 && pExceptionPtrs->ExceptionRecord != 0 &&
        pExceptionPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) 
    {
        // Special case to handle the stack overflow exception.
        // The dump will be realized from another thread.
        // Create another thread that will do the dump.
        HANDLE thread = ::CreateThread(0, 0,
            &StackOverflowThreadFunction, pExceptionPtrs, 0, 0);
        ::WaitForSingleObject(thread, INFINITE);
        ::CloseHandle(thread);
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }
    else
    {
        // Acquire lock to avoid other threads (if exist) to crash while we are inside
        LOCK_HANDLER();

        // Treat this type of crash critical by default
        GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

        // Generate error report.
        CR_EXCEPTION_INFO ei = {};
        ei.cb = sizeof(CR_EXCEPTION_INFO);
        ei.exctype = CR_SEH_EXCEPTION;
        ei.pexcptrs = pExceptionPtrs;
        GenerateErrorReport(&ei);

        if(!GetCurrentProcessCrashHandler()->bContinueExecution)
        {
            // Terminate process
            TerminateProcess(GetCurrentProcess(), 1);
        }

        // We do not unlock, because process is to be terminated.
        UNLOCK_HANDLER();
    }
    return EXCEPTION_EXECUTE_HANDLER;
}


// C++ terminate handler
static void TerminateHandler()
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Generate error report.
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_TERMINATE_CALL;
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

// C++ unexpected handler
static void UnexpectedHandler()
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_UNEXPECTED_CALL;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

#if _MSC_VER >= 1300
// C++ pure virtual call handler
static void PureCallHandler()
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_PURE_CALL;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}
#endif 

#if _MSC_VER >= 1300 && _MSC_VER < 1400
// Buffer overrun handler (deprecated in newest versions of Visual C++).
static void SecurityHandler(int code, void *x)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SECURITY_ERROR;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}
#endif

#if _MSC_VER >= 1400
// C++ Invalid parameter handler.
static void InvalidParameterHandler(const wchar_t* expression, 
                                    const wchar_t* function, 
                                    const wchar_t* file, 
                                    unsigned int line, 
                                    uintptr_t pReserved)
{
    UNREFERENCED_PARAMETER(pReserved);

    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_INVALID_PARAMETER;
    ei.expression = expression;
    ei.function = function;
    ei.file = file;
    ei.line = line; 

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}
#endif

#if _MSC_VER >= 1300
// C++ new operator fault (memory exhaustion) handler
static int NewHandler(size_t)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_NEW_OPERATOR_ERROR;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();

    // Unreacheable code
    return 0;
}
#endif

// Signal handlers
static void SigabrtHandler(int)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGABRT;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

static void SigfpeHandler(int code, int subcode)
{
    UNREFERENCED_PARAMETER(code);

    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGFPE;
    ei.pexcptrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;
    ei.fpe_subcode = subcode;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

static void SigintHandler(int)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGINT;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

static void SigillHandler(int)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGILL;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

static void SigsegvHandler(int)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGSEGV;
    ei.pexcptrs = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

static void SigtermHandler(int)
{
    // Acquire lock to avoid other threads (if exist) to crash while we are inside
    LOCK_HANDLER();

    // Treat this type of crash critical by default
    GetCurrentProcessCrashHandler()->bContinueExecution = FALSE;

    // Fill in the exception info
    CR_EXCEPTION_INFO ei = {};
    ei.cb = sizeof(CR_EXCEPTION_INFO);
    ei.exctype = CR_CPP_SIGTERM;

    // Generate crash report
    GenerateErrorReport(&ei);

    if(!GetCurrentProcessCrashHandler()->bContinueExecution)
    {
        // Terminate process
        TerminateProcess(GetCurrentProcess(), 1);
    }

    // We do not unlock, because process is to be terminated.
    UNLOCK_HANDLER();
}

//////////////////////////////////////////////////////////////////////////

// Create Minidump file
static bool CreateMiniDump(EXCEPTION_POINTERS* ep)
{
    MINIDUMP_EXCEPTION_INFORMATION mei = {};
    mei.ThreadId = ::GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = TRUE;

    // Make a name
    char szFileName[MAX_PATH];    
    const std::string& strModule = GetAppName();
    time_t now = time(NULL);
    tm thisDate = *localtime(&now);
    sprintf_s(szFileName, MAX_PATH, ("%s_%4d%02d%02d-%02d%02d%02d.dmp"), strModule.c_str(), 
        thisDate.tm_year+1900, thisDate.tm_mon+1, thisDate.tm_mday, thisDate.tm_hour,
        thisDate.tm_min, thisDate.tm_sec);

    // Create Minidump file
    HANDLE hFile = ::CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        GetDbghelpDll().MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            hFile, MiniDumpNormal, &mei, NULL, NULL);
        CloseHandle(hFile);
        return true;
    }
    else
    {
        LogLastError();
        return false;
    }
}


int GenerateErrorReport(PCR_EXCEPTION_INFO pExceptionInfo /*= NULL*/)
{
    // Only handle first chance exception in current thread
    static int excpt_chance = 0;
    if (++excpt_chance == 2)
    {
        return 1;
    }

    // Allocate memory in stack for storing exception pointers.
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextRecord;    
    EXCEPTION_POINTERS excptr;
    excptr.ExceptionRecord = &ExceptionRecord;
    excptr.ContextRecord = &ContextRecord; 

    // Get exception pointers if they were not provided by the caller. 
    if(pExceptionInfo->pexcptrs==NULL)
    {
        GetExceptionPointers(pExceptionInfo->code, &excptr);
        pExceptionInfo->pexcptrs = &excptr;
    }

    CreateMiniDump(&excptr);
    CreateReport(&excptr);

    return 0;
}

//////////////////////////////////////////////////////////////////////////
int SetProcessExceptionHanlders(DWORD dwFlags)
{
    ProcessExceptHandlder prevHandlers = {};

    // If 0 is specified as dwFlags, assume all handlers should be
    // installed
    if((dwFlags & CR_INST_ALL_POSSIBLE_HANDLERS) == 0)
    {
        dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
    }

    if(dwFlags & CR_INST_STRUCTURED_EXCEPTION_HANDLER)
    {
        // Install top-level SEH handler
        prevHandlers.pfnSehHandler = SetUnhandledExceptionFilter(SehHandler);
    }

    _set_error_mode(_OUT_TO_STDERR);

#if _MSC_VER >= 1300
    if(dwFlags & CR_INST_PURE_CALL_HANDLER)
    {
        // Catch pure virtual function calls.
        // Because there is one _purecall_handler for the whole process, 
        // calling this function immediately impacts all threads. The last 
        // caller on any thread sets the handler. 
        // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
        prevHandlers.pfnPurec = _set_purecall_handler(PureCallHandler);    
    }

    if(dwFlags & CR_INST_NEW_OPERATOR_ERROR_HANDLER)
    {
        // Catch new operator memory allocation exceptions
        _set_new_mode(1); // Force malloc() to call new handler too
        prevHandlers.pfnNewHandler = _set_new_handler(NewHandler);
    }
#endif

#if _MSC_VER >= 1400
    if(dwFlags & CR_INST_INVALID_PARAMETER_HANDLER)
    {
        // Catch invalid parameter exceptions.
        prevHandlers.pfnInvpar = _set_invalid_parameter_handler(InvalidParameterHandler); 
    }
#endif

#if _MSC_VER >= 1300 && _MSC_VER < 1400    
    if(dwFlags & CR_INST_SECURITY_ERROR_HANDLER)
    {
        // Catch buffer overrun exceptions
        // The _set_security_error_handler is deprecated in VC8 C++ run time library
        prevHandlers.pfnvSec = _set_security_error_handler(SecurityHandler);
    }
#endif

    // Set up C++ signal handlers
    if(dwFlags & CR_INST_SIGABRT_HANDLER)
    {
#if _MSC_VER >= 1400  
        _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif
        // Catch an abnormal program termination
        prevHandlers.pfnSigabrtHandler = signal(SIGABRT, SigabrtHandler);  
    }

    if(dwFlags & CR_INST_SIGINT_HANDLER)
    {
        // Catch illegal instruction handler
        prevHandlers.pfnSigintHandler = signal(SIGINT, SigintHandler);     
    }

    if(dwFlags & CR_INST_TERMINATE_HANDLER)
    {
        // Catch a termination request
        prevHandlers.pfnSigtermHandler = signal(SIGTERM, SigtermHandler);          
    }

    if(dwFlags & CR_INST_SIGFPE_HANDLER)
    {
        // Catch a floating point error
        typedef void (*sigh)(int);
        typedef void (*sigfpe)(int,int);
        prevHandlers.pfnSigfpeHandler = (sigfpe)signal(SIGFPE, (sigh)SigfpeHandler);     
    }


    if(dwFlags & CR_INST_SIGILL_HANDLER)
    {
        // Catch an illegal instruction
        prevHandlers.pfnSigillHandler = signal(SIGILL, SigillHandler);     
    }

    if(dwFlags & CR_INST_SIGSEGV_HANDLER)
    {
        // Catch illegal storage access errors
        prevHandlers.pfnSigsegvHandler = signal(SIGSEGV, SigsegvHandler);   
    }

    if(dwFlags & CR_INST_TERMINATE_HANDLER)
    {
        // Catch terminate() calls. 
        // In a multithreaded environment, terminate functions are maintained 
        // separately for each thread. Each new thread needs to install its own 
        // terminate function. Thus, each thread is in charge of its own termination handling.
        // http://msdn.microsoft.com/en-us/library/t6fk7h29.aspx
        prevHandlers.pfnTerminateHandler = set_terminate(TerminateHandler);       
    }

    if(dwFlags & CR_INST_UNEXPECTED_HANDLER)
    {
        // Catch unexpected() calls.
        // In a multithreaded environment, unexpected functions are maintained 
        // separately for each thread. Each new thread needs to install its own 
        // unexpected function. Thus, each thread is in charge of its own unexpected handling.
        // http://msdn.microsoft.com/en-us/library/h46t5b69.aspx  
        prevHandlers.pfnUnexpectedHandler = set_unexpected(UnexpectedHandler);    
    }

    return TRUE;
}

