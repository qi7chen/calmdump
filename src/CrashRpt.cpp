/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

// File: CrashRpt.cpp
// Description: CrashRpt API implementation.
// Authors: mikecarruth, zexspectrum
// Date: 

//////////////////////////////////////////////////////////////////////////
// @file    CrashRpt.cpp
// @author  ichenq@gmail.com 
// @date    Jul, 2013
// @brief  
//////////////////////////////////////////////////////////////////////////

#include "CrashRpt.h"
#include <float.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <eh.h>
#include "CrashHandler.h"

int crInstall()
{
    SetProcessExceptionHanlders();
    return 0;
}

int crUninstall()
{
    return 0;
}

int crExceptionFilter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
{
    CR_EXCEPTION_INFO ei;
    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
    ei.cb = sizeof(CR_EXCEPTION_INFO);  
    ei.exctype = CR_SEH_EXCEPTION;
    ei.pexcptrs = ep;
    ei.code = code;

    int res = GenerateErrorReport(&ei);
    if(res!=0)
    {
        // If goes here than GenerateErrorReport() failed  
        return EXCEPTION_CONTINUE_SEARCH;  
    }  

    return EXCEPTION_EXECUTE_HANDLER;  
}

//-----------------------------------------------------------------------------------------------
// Below crEmulateCrash() related stuff goes 


class CDerived;
class CBase
{
public:
    CBase(CDerived *derived): m_pDerived(derived) {};
    ~CBase();
    virtual void function(void) = 0;

    CDerived * m_pDerived;
};

class CDerived : public CBase
{
public:
    CDerived() : CBase(this) {};   // C4355
    virtual void function(void) {};
};

CBase::~CBase()
{
    m_pDerived->function();
}


void sigfpe_test()
{ 
    // Code taken from http://www.devx.com/cplus/Article/34993/1954

    //Set the x86 floating-point control word according to what
    //exceptions you want to trap. 
    _clearfp(); //Always call _clearfp before setting the control
    //word
    //Because the second parameter in the following call is 0, it
    //only returns the floating-point control word
    unsigned int cw; 
#if _MSC_VER<1400
    cw = _controlfp(0, 0); //Get the default control
#else
    _controlfp_s(&cw, 0, 0); //Get the default control
#endif 
    //word
    //Set the exception masks off for exceptions that you want to
    //trap.  When a mask bit is set, the corresponding floating-point
    //exception is //blocked from being generating.
    cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_ZERODIVIDE|
        EM_DENORMAL|EM_INVALID);
    //For any bit in the second parameter (mask) that is 1, the 
    //corresponding bit in the first parameter is used to update
    //the control word.  
    unsigned int cwOriginal = 0;
#if _MSC_VER<1400
    cwOriginal = _controlfp(cw, MCW_EM); //Set it.
#else
    _controlfp_s(&cwOriginal, cw, MCW_EM); //Set it.
#endif
    //MCW_EM is defined in float.h.

    // Divide by zero

    float a = 1.0f;
    float b = 0.0f;
    float c = a/b;
    c;

    //Restore the original value when done:
    //_controlfp_s(cwOriginal, MCW_EM);
}

#define BIG_NUMBER 0x1fffffff
//#define BIG_NUMBER 0xf
#pragma warning(disable: 4717) // avoid C4717 warning
int RecurseAlloc() 
{
    int *pi = NULL;
    for(;;)
        pi = new int[BIG_NUMBER];
    return 0;
}

// Vulnerable function
#pragma warning(disable : 4996)   // for strcpy use
#pragma warning(disable : 6255)   // warning C6255: _alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead
#pragma warning(disable : 6204)   // warning C6204: Possible buffer overrun in call to 'strcpy': use of unchecked parameter 'str'
void test_buffer_overrun(const char *str) 
{
    char* buffer = (char*)_alloca(10);
    strcpy(buffer, str); // overrun buffer !!!

    // use a secure CRT function to help prevent buffer overruns
    // truncate string to fit a 10 byte buffer
    // strncpy_s(buffer, _countof(buffer), str, _TRUNCATE);
}
#pragma warning(default : 4996)  
#pragma warning(default : 6255)   
#pragma warning(default : 6204)   

// Stack overflow function
struct DisableTailOptimization
{
    ~DisableTailOptimization() {
        ++ v;
    }
    static int v;
};

int DisableTailOptimization::v = 0;

static void CauseStackOverflow()
{
    DisableTailOptimization v;
    CauseStackOverflow();
}

int crEmulateCrash(unsigned ExceptionType)
{
    switch(ExceptionType)
    {
    case CR_SEH_EXCEPTION:
        {
            // Access violation
            int *p = 0;
#pragma warning(disable : 6011)   // warning C6011: Dereferencing NULL pointer 'p'
            *p = 0;
#pragma warning(default : 6011)   
        }
        break;
    case CR_CPP_TERMINATE_CALL:
        {
            // Call terminate
            terminate();
        }
        break;
    case CR_CPP_UNEXPECTED_CALL:
        {
            // Call unexpected
            unexpected();
        }
        break;
    case CR_CPP_PURE_CALL:
        {
            // pure virtual method call
            CDerived derived;
        }
        break;
    case CR_CPP_SECURITY_ERROR:
        {
            // Cause buffer overrun (/GS compiler option)

            // declare buffer that is bigger than expected
            char large_buffer[] = "This string is longer than 10 characters!!";
            test_buffer_overrun(large_buffer);
        }
        break;
    case CR_CPP_INVALID_PARAMETER:
        {      
            char* formatString;
            // Call printf_s with invalid parameters.
            formatString = NULL;
            // warning C6387: 'argument 1' might be '0': this does not adhere to the specification for the function 'printf'
#pragma warning(disable : 6387) 
            printf(formatString);
#pragma warning(default : 6387)   

        }
        break;
    case CR_CPP_NEW_OPERATOR_ERROR:
        {
            // Cause memory allocation error
            RecurseAlloc();
        }
        break;
    case CR_CPP_SIGABRT: 
        {
            // Call abort
            abort();
        }
        break;
    case CR_CPP_SIGFPE:
        {
            // floating point exception ( /fp:except compiler option)
            sigfpe_test();
            return 1;
        }    
    case CR_CPP_SIGILL: 
        {
            int result = raise(SIGILL);
            assert(result==0);
            return result;
        }    
    case CR_CPP_SIGINT: 
        {
            int result = raise(SIGINT);  
            assert(result==0);
            return result;
        }    
    case CR_CPP_SIGSEGV: 
        {
            int result = raise(SIGSEGV);
            assert(result==0);
            return result;
        }    
    case CR_CPP_SIGTERM: 
        {
            int result = raise(SIGTERM);
            assert(result==0);
            return result;
        }
    case CR_NONCONTINUABLE_EXCEPTION: 
        {
            // Raise noncontinuable software exception
            RaiseException(123, EXCEPTION_NONCONTINUABLE, 0, NULL);        
        }
        break;
    case CR_THROW: 
        {
            // Throw typed C++ exception.
            throw 13;
        }
        break;
    case CR_STACK_OVERFLOW:
        {
            // Infinite recursion and stack overflow.
            CauseStackOverflow();						
        }
    default:
        break;
    }

    return 1;
}

