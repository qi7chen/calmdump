/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

// File: Utility.cpp
// Description: Miscellaneous helper functions
// Authors: mikecarruth, zexspectrum
// Date: 

//////////////////////////////////////////////////////////////////////////
// @file    Utility.cpp
// @author  ichenq@gmail.com 
// @date    Jul, 2013
// @brief  
//////////////////////////////////////////////////////////////////////////

#include "Utility.h"
#include <Tlhelp32.h>



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


// The following code gets exception pointers using a workaround found in CRT code.
void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS* pExceptionPointers)
{
    // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)
    CONTEXT ContextRecord;
    memset(&ContextRecord, 0, sizeof(CONTEXT));

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
    RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

    ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

    memcpy(pExceptionPointers->ContextRecord, &ContextRecord, sizeof(CONTEXT));

    ZeroMemory(pExceptionPointers->ExceptionRecord, sizeof(EXCEPTION_RECORD));

    pExceptionPointers->ExceptionRecord->ExceptionCode = dwExceptionCode;
    pExceptionPointers->ExceptionRecord->ExceptionAddress = _ReturnAddress();   
}


void SuspendOtherThreads()
{
    DWORD dwProcessId = ::GetCurrentProcessId();
    DWORD dwThreadId = ::GetCurrentThreadId();
    HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return ;
    }
    
    THREADENTRY32 entry = {};
    entry.dwSize = sizeof(entry);
    if (!::Thread32First(hSnapshot, &entry))
    {
        ::CloseHandle(hSnapshot);
        return ;
    }

    do {
        if (entry.th32OwnerProcessID == dwProcessId && entry.th32ThreadID != dwThreadId)
        {
            HANDLE hThread = ::OpenThread(THREAD_SUSPEND_RESUME, FALSE, entry.th32ThreadID);
            if (hThread != NULL)
            {
                ::SuspendThread(hThread);
            }
        }
    }while (::Thread32Next(hSnapshot, &entry));

    CloseHandle(hSnapshot);
}