/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

// File: CrashHandler.h
// Description: Exception handling functionality.
// Authors: mikecarruth, zexspectrum
// Date: 2009

//////////////////////////////////////////////////////////////////////////
// @file    CrashHandler.h 
// @author  ichenq@gmail.com 
// @date    Jul, 2013
// @brief  
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>
#include <stdlib.h>
#include <new.h>
#include <exception>
#include "CrashRpt.h"
#include "Utility.h"


/* This structure contains pointer to the exception handlers for a process.*/
struct ProcessExceptHandlder
{
    // Previous SEH exception filter.
    LPTOP_LEVEL_EXCEPTION_FILTER  pfnSehHandler;

    // C++ terminate handler
    terminate_function  pfnTerminateHandler;

    // C++ unexpected handler
    unexpected_function pfnUnexpectedHandler;

#if _MSC_VER >= 1300
    _purecall_handler   pfnPurec;       // Pure virtual call exception filter.
    _PNH    pfnNewHandler;              // New operator exception filter.
#endif

#if _MSC_VER >= 1400
    _invalid_parameter_handler pfnInvpar; // Invalid parameter exception filter.
#endif

#if _MSC_VER >= 1300 && _MSC_VER < 1400
    _secerr_handler_func    pfnvSec;   // security exception filter.
#endif

    // Signal handlers
    void (*pfnSigabrtHandler)(int);           // SIGABRT signal handler
    void (*pfnSigfpeHandler)(int, int);       // FPE handler
    void (*pfnSigintHandler)(int);            // Illegal instruction handler
    void (*pfnSigillHandler)(int);            // SIGILL handler
    void (*pfnSigsegvHandler)(int);           // Illegal storage access handler
    void (*pfnSigtermHandler)(int);           // Termination request handler
};



struct CurrentProcessCrashHandler
{
    // Avoid other threads (if exist) to crash while we are inside. 
    CritcalSection          critsec;

    // Previous process exception handlers
    ProcessExceptHandlder   prevHandlers;

    // Install information
    CR_INSTALL_INFO         installInfo;

    // Whether to terminate process (the default) or to continue execution after crash.
    BOOL                    bContinueExecution;
};


CurrentProcessCrashHandler* GetCurrentProcessCrashHandler();

#define LOCK_HANDLER()      GetCurrentProcessCrashHandler()->critsec.Lock()
#define UNLOCK_HANDLER()    GetCurrentProcessCrashHandler()->critsec.Unlock()


// Generates error report	
int GenerateErrorReport(PCR_EXCEPTION_INFO pExceptionInfo = NULL);