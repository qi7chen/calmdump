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

// @file    CrashHandler.h 
// @author  ichenq@gmail.com 
// @date    Jul, 2013
// @brief  

#include "CrashHandler.h"
#include "CrashRpt.h"
#include <signal.h>

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



//////////////////////////////////////////////////////////////////////////
// Exception handler functions. 
//


static DWORD WINAPI StackOverflowThreadFunction(LPVOID lpParameter)
{
    PEXCEPTION_POINTERS pExceptionPtrs = reinterpret_cast<PEXCEPTION_POINTERS>(lpParameter);
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
    return EXCEPTION_EXECUTE_HANDLER;
}


// C++ terminate handler
static void TerminateHandler()
{

}

// C++ unexpected handler
static void UnexpectedHandler()
{

}

#if _MSC_VER >= 1300
// C++ pure virtual call handler
static void PureCallHandler()
{

}
#endif 

#if _MSC_VER >= 1300 && _MSC_VER < 1400
// Buffer overrun handler (deprecated in newest versions of Visual C++).
static void SecurityHandler(int code, void *x)
{

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

}
#endif

#if _MSC_VER >= 1300
// C++ new operator fault (memory exhaustion) handler
static int NewHandler(size_t)
{
    return 0;
}
#endif

// Signal handlers
static void SigabrtHandler(int)
{

}

static void SigfpeHandler(int, int)
{

}

static void SigintHandler(int)
{

}

static void SigillHandler(int)
{

}

static void SigsegvHandler(int)
{

}

static void SigtermHandler(int)
{

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
