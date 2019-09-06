/************************************************************************************* 
  This file is a part of CrashRpt library.

  Copyright (c) 2003, Michael Carruth
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:
 
   * Redistributions of source code must retain the above copyright notice, this 
     list of conditions and the following disclaimer.
 
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
 
   * Neither the name of the author nor the names of its contributors 
     may be used to endorse or promote products derived from this software without 
     specific prior written permission.
 

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************************/

/*! \file   CrashRpt.h
 *  \brief  Defines the interface for the CrashRpt.DLL.
 *  \date   2003-2011
 *  \author Michael Carruth 
 *  \author zeXspectrum 
 */

// Copyright (C) 2015 prototyped.cn All rights reserved.
// Distributed under the terms and conditions of the Apache License. 
// See accompanying files LICENSE.

#pragma once

#include <windows.h>


// Define SAL macros to be empty if some old Visual Studio used
#ifndef __reserved
  #define __reserved
#endif
#ifndef __in
  #define __in
#endif
#ifndef __in_opt
  #define __in_opt
#endif
#ifndef __out_ecount_z
  #define __out_ecount_z(x)
#endif

#ifdef __cplusplus
#define CRASHRPT_EXTERNC extern "C"
#else
#define CRASHRPT_EXTERNC
#endif

#define CRASHRPTAPI(rettype) CRASHRPT_EXTERNC rettype WINAPI


// Flags for CR_INSTALL_INFO::dwFlags
#define CR_INST_STRUCTURED_EXCEPTION_HANDLER      0x1 //!< Install SEH handler (deprecated name, use \ref CR_INST_SEH_EXCEPTION_HANDLER instead).
#define CR_INST_SEH_EXCEPTION_HANDLER             0x1 //!< Install SEH handler.
#define CR_INST_TERMINATE_HANDLER                 0x2 //!< Install terminate handler.
#define CR_INST_UNEXPECTED_HANDLER                0x4 //!< Install unexpected handler.
#define CR_INST_PURE_CALL_HANDLER                 0x8 //!< Install pure call handler (VS .NET and later).
#define CR_INST_NEW_OPERATOR_ERROR_HANDLER       0x10 //!< Install new operator error handler (VS .NET and later).
#define CR_INST_SECURITY_ERROR_HANDLER           0x20 //!< Install security error handler (VS .NET and later).
#define CR_INST_INVALID_PARAMETER_HANDLER        0x40 //!< Install invalid parameter handler (VS 2005 and later).
#define CR_INST_SIGABRT_HANDLER                  0x80 //!< Install SIGABRT signal handler.
#define CR_INST_SIGFPE_HANDLER                  0x100 //!< Install SIGFPE signal handler.   
#define CR_INST_SIGILL_HANDLER                  0x200 //!< Install SIGILL signal handler.  
#define CR_INST_SIGINT_HANDLER                  0x400 //!< Install SIGINT signal handler.  
#define CR_INST_SIGSEGV_HANDLER                 0x800 //!< Install SIGSEGV signal handler.
#define CR_INST_SIGTERM_HANDLER                0x1000 //!< Install SIGTERM signal handler.  

#define CR_INST_ALL_POSSIBLE_HANDLERS          0x1FFF //!< Install all possible exception handlers.
#define CR_INST_CRT_EXCEPTION_HANDLERS         0x1FFE //!< Install exception handlers for the linked CRT module.

#define CR_INST_NO_GUI                         0x2000 //!< Do not show GUI, send report silently (use for non-GUI apps only).
#define CR_INST_HTTP_BINARY_ENCODING           0x4000 //!< Deprecated, do not use.
#define CR_INST_DONT_SEND_REPORT               0x8000 //!< Don't send error report immediately, just save it locally.
#define CR_INST_APP_RESTART                   0x10000 //!< Restart the application on crash.
#define CR_INST_NO_MINIDUMP                   0x20000 //!< Do not include minidump file to crash report.
#define CR_INST_SEND_QUEUED_REPORTS           0x40000 //!< CrashRpt should send error reports that are waiting to be delivered.
#define CR_INST_STORE_ZIP_ARCHIVES            0x80000 //!< CrashRpt should store both uncompressed error report files and ZIP archives.
#define CR_INST_SEND_MANDATORY				 0x100000 //!< This flag removes the "Close" and "Other actions" buttons from Error Report dialog, thus making the sending procedure mandatory for user.
#define CR_INST_SHOW_ADDITIONAL_INFO_FIELDS	 0x200000 //!< Makes "Your E-mail" and "Describe what you were doing when the problem occurred" fields of Error Report dialog always visible.
#define CR_INST_ALLOW_ATTACH_MORE_FILES		 0x400000 //!< Adds an ability for user to attach more files to crash report by clicking "Attach More File(s)" item from context menu of Error Report Details dialog.
#define CR_INST_AUTO_THREAD_HANDLERS         0x800000 //!< If this flag is set, installs exception handlers for newly created threads automatically.


/*! \ingroup CrashRptAPI 
 *  \brief  Installs exception handlers for the caller process.
 *
 *
 *  \remarks
 *    This function installs unhandled exception filter for the caller process.
 *    It also installs various CRT exception/error handlers that function for all threads of the caller process.
 *    For more information, see \ref exception_handling
 *
 *    Below is the list of installed handlers:
 *     - Top-level SEH exception filter [ \c SetUnhandledExceptionFilter() ]
 *     - C++ pure virtual call handler (Visual Studio .NET 2003 and later) [ \c _set_purecall_handler() ]
 *     - C++ invalid parameter handler (Visual Studio .NET 2005 and later) [ \c _set_invalid_parameter_handler() ]
 *     - C++ new operator error handler (Visual Studio .NET 2003 and later) [ \c _set_new_handler() ]
 *     - C++ buffer overrun handler (Visual Studio .NET 2003 only) [ \c _set_security_error_handler() ]
 *     - C++ abort handler [ \c signal(SIGABRT) ]
 *     - C++ illegal instruction handler [ \c signal(SIGINT) ]
 *     - C++ termination request [ \c signal(SIGTERM) ]
 *
 *    In a multithreaded program, additionally use crInstallToCurrentThread2() function for each execution
 *    thread, except the main one.
 * 
 */
int crInstall();


/*! \ingroup CrashRptAPI 
 *  \brief Unsinstalls exception handlers previously installed with crInstall().
 *
 *  \return
 *    This function returns zero if succeeded.
 *
 *  \remarks
 *
 *    Call this function on application exit to uninstall exception
 *    handlers previously installed with crInstall(). After function call, the exception handlers
 *    are restored to states they had before calling crInstall().
 *
 *    This function fails if crInstall() wasn't previously called in context of the
 *    caller process.
 *
 *    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
 *
 *  \sa crInstallW(), crInstallA(), crInstall(), crUninstall(),
 *      CrAutoInstallHelper
 */
int crUninstall();


/*! \ingroup CrashRptAPI
*  \brief Installs exception handlers to the caller thread.
*  \return This function returns zero if succeeded.
*  \param[in] dwFlags Flags.
*
*  \remarks
*
*  This function is available <b>since v.1.1.2</b>.
*
*  The function sets exception handlers for the caller thread. If you have
*  several execution threads, you ought to call the function for each thread,
*  except the main one.
*   
*  \a dwFlags defines what exception handlers to install. Use zero value
*  to install all possible exception handlers. Or use a combination of the following constants:
*
*      - \ref CR_INST_TERMINATE_HANDLER              Install terminate handler
*      - \ref CR_INST_UNEXPECTED_HANDLER             Install unexpected handler
*      - \ref CR_INST_SIGFPE_HANDLER                 Install SIGFPE signal handler   
*      - \ref CR_INST_SIGILL_HANDLER                 Install SIGILL signal handler  
*      - \ref CR_INST_SIGSEGV_HANDLER                Install SIGSEGV signal handler 
* 
*  Example:
*
*   \code
*   DWORD WINAPI ThreadProc(LPVOID lpParam)
*   {
*     // Install exception handlers
*     crInstallToCurrentThread2(0);
*
*     // Your code...
*
*     // Uninstall exception handlers
*     crUninstallFromCurrentThread();
*    
*     return 0;
*   }
*   \endcode
* 
*  \sa 
*    crInstall()
*/
int crInstallToCurrentThread2(DWORD dwFlags);

/*! \ingroup CrashRptAPI  
*  \brief Uninstalls C++ exception handlers from the current thread.
*  \return This function returns zero if succeeded.
*  
*  \remarks
*
*    This function unsets exception handlers from the caller thread. If you have
*    several execution threads, you ought to call the function for each thread.
*    After calling this function, the exception handlers for current thread are
*    replaced with the handlers that were before call of crInstallToCurrentThread2().
*
*    This function fails if crInstallToCurrentThread2() wasn't called for current thread.
*
*    When this function fails, use crGetLastErrorMsg() to retrieve the error message.
*
*    No need to call this function for the main execution thread. The crUninstall()
*    will automatically uninstall C++ exception handlers for the main thread.
*
*   \sa crInstallToCurrentThread2(),
*       crUninstallFromCurrentThread(), CrThreadAutoInstallHelper
*/

int crUninstallFromCurrentThread();

// Exception types
#define CR_WIN32_STRUCTURED_EXCEPTION   0    //!< SEH exception (deprecated name, use \ref CR_SEH_EXCEPTION instead).
#define CR_SEH_EXCEPTION                0    //!< SEH exception.
#define CR_CPP_TERMINATE_CALL           1    //!< C++ terminate() call.
#define CR_CPP_UNEXPECTED_CALL          2    //!< C++ unexpected() call.
#define CR_CPP_PURE_CALL                3    //!< C++ pure virtual function call (VS .NET and later).
#define CR_CPP_NEW_OPERATOR_ERROR       4    //!< C++ new operator fault (VS .NET and later).
#define CR_CPP_SECURITY_ERROR           5    //!< Buffer overrun error (VS .NET only).
#define CR_CPP_INVALID_PARAMETER        6    //!< Invalid parameter exception (VS 2005 and later).
#define CR_CPP_SIGABRT                  7    //!< C++ SIGABRT signal (abort).
#define CR_CPP_SIGFPE                   8    //!< C++ SIGFPE signal (flotating point exception).
#define CR_CPP_SIGILL                   9    //!< C++ SIGILL signal (illegal instruction).
#define CR_CPP_SIGINT                   10   //!< C++ SIGINT signal (CTRL+C).
#define CR_CPP_SIGSEGV                  11   //!< C++ SIGSEGV signal (invalid storage access).
#define CR_CPP_SIGTERM                  12   //!< C++ SIGTERM signal (termination request).


/*! \ingroup CrashRptStructs
 *  
 *  This structure defines the information needed to generate crash minidump file and
 *  provide the developer with other information about the error. This structure is used by
 *  the crGenerateErrorReport() function.
 *
 *  \b cb [in] 
 *
 *  This must contain the size of this structure in bytes.
 *
 *  \b pexcptrs [in, optional]
 *
 *    Should contain the exception pointers. If this parameter is NULL, 
 *    the current CPU state is used to generate exception pointers.
 *
 *  \b exctype [in]
 *  
 *    The type of exception. This parameter may be one of the following:
 *     - \ref CR_SEH_EXCEPTION             SEH (Structured Exception Handling) exception
 *     - \ref CR_CPP_TERMINATE_CALL        C++ terminate() function call
 *     - \ref CR_CPP_UNEXPECTED_CALL       C++ unexpected() function call
 *     - \ref CR_CPP_PURE_CALL             Pure virtual method call (Visual Studio .NET 2003 and later) 
 *     - \ref CR_CPP_NEW_OPERATOR_ERROR    C++ 'new' operator error (Visual Studio .NET 2003 and later)
 *     - \ref CR_CPP_SECURITY_ERROR        Buffer overrun (Visual Studio .NET 2003 only) 
 *     - \ref CR_CPP_INVALID_PARAMETER     Invalid parameter error (Visual Studio 2005 and later) 
 *     - \ref CR_CPP_SIGABRT               C++ SIGABRT signal 
 *     - \ref CR_CPP_SIGFPE                C++ floating point exception
 *     - \ref CR_CPP_SIGILL                C++ illegal instruction
 *     - \ref CR_CPP_SIGINT                C++ SIGINT signal
 *     - \ref CR_CPP_SIGSEGV               C++ invalid storage access
 *     - \ref CR_CPP_SIGTERM               C++ termination request
 * 
 *   \b code [in, optional]
 *
 *      Used if \a exctype is \ref CR_SEH_EXCEPTION and represents the SEH exception code. 
 *      If \a pexptrs is NULL, this value is used when generating exception information for initializing
 *      \c pexptrs->ExceptionRecord->ExceptionCode member, otherwise it is ignored.
 *
 *   \b fpe_subcode [in, optional]
 *
 *      Used if \a exctype is equal to \ref CR_CPP_SIGFPE. It defines the floating point
 *      exception subcode (see \c signal() function ducumentation in MSDN).
 * 
 *   \b expression, \b function, \b file and \b line [in, optional]
 *
 *     These parameters are used when \a exctype is \ref CR_CPP_INVALID_PARAMETER. 
 *     These members are typically non-zero when using debug version of CRT.
 *
 *  \b bManual [in]
 *
 *     Since v.1.2.4, \a bManual parameter should be equal to TRUE if the report is generated manually. 
 *     The value of \a bManual parameter affects the automatic application restart behavior. If the application
 *     restart is requested by the \ref CR_INST_APP_RESTART flag of CR_INSTALL_INFO::dwFlags structure member, 
 *     and if \a bManual is FALSE, the application will be
 *     restarted after error report generation. If \a bManual is TRUE, the application won't be restarted.
 *
 *  \b hSenderProcess [out]
 *
 *     As of v.1.2.8, \a hSenderProcess parameter will contain the handle to the <b>CrashSender.exe</b> process when 
 *     \ref crGenerateErrorReport function returns. The caller may use this handle to wait until <b>CrashSender.exe</b> 
 *     process exits and check the exit code.
 */

typedef struct tagCR_EXCEPTION_INFO
{
  WORD cb;                   //!< Size of this structure in bytes; should be initialized before using.
  PEXCEPTION_POINTERS pexcptrs; //!< Exception pointers.
  int exctype;               //!< Exception type.
  DWORD code;                //!< Code of SEH exception.
  unsigned int fpe_subcode;  //!< Floating point exception subcode.
  const wchar_t* expression; //!< Assertion expression.
  const wchar_t* function;   //!< Function in which assertion happened.
  const wchar_t* file;       //!< File in which assertion happened.
  unsigned int line;         //!< Line number.
  BOOL bManual;              //!< Flag telling if the error report is generated manually or not.
  HANDLE hSenderProcess;     //!< Handle to the CrashSender.exe process.
}
CR_EXCEPTION_INFO;

typedef CR_EXCEPTION_INFO *PCR_EXCEPTION_INFO;


/*! \ingroup DeprecatedAPI 
 *  \brief Can be used as a SEH exception filter.
 *
 *  \return This function returns \c EXCEPTION_EXECUTE_HANDLER if succeeds, else \c EXCEPTION_CONTINUE_SEARCH.
 *
 *  \param[in] code Exception code.
 *  \param[in] ep   Exception pointers.
 *
 *  \remarks
 *
 *     As of v.1.2.8, this function is declared deprecated. It may be removed in one of the future releases.
 *
 *     This function can be called instead of a SEH exception filter
 *     inside of __try{}__except(Expression){} statement. The function generates a error report
 *     and returns control to the exception handler block.
 *
 *     The exception code is usually retrieved with \b GetExceptionCode() intrinsic function
 *     and the exception pointers are retrieved with \b GetExceptionInformation() intrinsic 
 *     function.
 *
 *     If an error occurs, this function returns \c EXCEPTION_CONTINUE_SEARCH.
 *     Use crGetLastErrorMsg() to retrieve the error message on fail.
 *
 *     The following example shows how to use crExceptionFilter().
 *    
 *     \code
 *     int* p = NULL;   // pointer to NULL
 *     __try
 *     {
 *        *p = 13; // causes an access violation exception;
 *     }
 *     __except(crExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
 *     {   
 *       // Terminate program
 *       ExitProcess(1);
 *     }
 *
 *     \endcode 
 */
int crExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS* ep);

// Flags used by crEmulateCrash() function
#define CR_NONCONTINUABLE_EXCEPTION  32  //!< Non continuable sofware exception. 
#define CR_THROW                     33  //!< Throw C++ typed exception.
#define CR_STACK_OVERFLOW			 34  //!< Stack overflow.

/*! \ingroup CrashRptAPI  
 *  \brief Emulates a predefined crash situation.
 *
 *  \return This function doesn't return if succeded. If failed, returns non-zero value. Call crGetLastErrorMsg()
 *   to get the last error message.
 *
 *  \param[in] ExceptionType Type of crash.
 *
 *  \remarks
 *
 *    This function uses some a priori incorrect or vulnerable code or raises a C++ signal or raises an uncontinuable
 *    software exception to cause crash.
 *
 *    This function can be used to test if CrashRpt handles a crash situation correctly.
 *    
 *    CrashRpt will intercept an error or exception if crInstall() and/or crInstallToCurrentThread2() 
 *    were previously called. crInstall() installs exception handlers that function on per-process basis.
 *    crInstallToCurrentThread2() installs exception handlers that function on per-thread basis.
 *    
 *  \a ExceptionType can be one of the following constants:
 *    - \ref CR_SEH_EXCEPTION  This will generate a null pointer exception.
 *    - \ref CR_CPP_TERMINATE_CALL This results in call of terminate() C++ function.
 *    - \ref CR_CPP_UNEXPECTED_CALL This results in call of unexpected() C++ function.
 *    - \ref CR_CPP_PURE_CALL This emulates a call of pure virtual method call of a C++ class instance (Visual Studio .NET 2003 and later).
 *    - \ref CR_CPP_NEW_OPERATOR_ERROR This emulates C++ new operator failure (Visual Studio .NET 2003 and later).
 *    - \ref CR_CPP_SECURITY_ERROR This emulates copy of large amount of data to a small buffer (Visual Studio .NET 2003 only).
 *    - \ref CR_CPP_INVALID_PARAMETER This emulates an invalid parameter C++ exception (Visual Studio 2005 and later). 
 *    - \ref CR_CPP_SIGABRT This raises SIGABRT signal (abnormal program termination).
 *    - \ref CR_CPP_SIGFPE This causes floating point exception.
 *    - \ref CR_CPP_SIGILL This raises SIGILL signal (illegal instruction signal).
 *    - \ref CR_CPP_SIGINT This raises SIGINT signal.
 *    - \ref CR_CPP_SIGSEGV This raises SIGSEGV signal.
 *    - \ref CR_CPP_SIGTERM This raises SIGTERM signal (program termination request).
 *    - \ref CR_NONCONTINUABLE_EXCEPTION This raises a noncontinuable software exception (expected result 
 *         is the same as in \ref CR_SEH_EXCEPTION).
 *    - \ref CR_THROW This throws a C++ typed exception (expected result is the same as in \ref CR_CPP_TERMINATE_CALL).
 *
 *  The \ref CR_SEH_EXCEPTION uses null pointer write operation to cause the access violation.
 *
 *  The \ref CR_NONCONTINUABLE_EXCEPTION has the same effect as \ref CR_SEH_EXCEPTION, but it uses
 *  \b RaiseException() WinAPI function to raise noncontinuable software exception.
 *
 *  The following example shows how to use crEmulateCrash() function.
 *
 *  \code
 *  // emulate null pointer exception (access violation)
 *  crEmulateCrash(CR_SEH_EXCEPTION);
 *  \endcode
 *
 */
int crEmulateCrash(unsigned ExceptionType) throw (...);




//// Helper wrapper classes

#ifndef _CRASHRPT_NO_WRAPPERS

/*! \class CrAutoInstallHelper
 *  \ingroup CrashRptWrappers
 *  \brief Installs exception handlers in constructor and uninstalls in destructor.
 *  \remarks
 *    Use this class to easily install/uninstall exception handlers in you \b main()
 *    or \b WinMain() function.
 *
 *    This wrapper class calls crInstall() in its constructor and calls crUninstall() in
 *    its destructor.
 *
 *    Use CrAutoInstallHelper::m_nInstallStatus member to check the return status of crInstall().
 *   
 */

class CrAutoInstallHelper
{
public:

  //! Installs exception handlers to the caller process
  CrAutoInstallHelper()
  {
    m_nInstallStatus = crInstall();
  }


  //! Uninstalls exception handlers from the caller process
  ~CrAutoInstallHelper()
  {
    crUninstall();
  }

  //! Install status
  int m_nInstallStatus;
};


#endif //!_CRASHRPT_NO_WRAPPERS




