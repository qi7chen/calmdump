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
#include <stdio.h>
#include <assert.h>

#pragma warning(disable: 4996)


// Returns mudule name of the launched current process.
std::string GetModuleName(HMODULE hModule)
{
    char szFileName[MAX_PATH];
    GetModuleFileNameA(hModule, szFileName, _MAX_FNAME);
    std::string sAppName = szFileName;
    size_t start = sAppName.find_last_of('\\') + 1;
    return sAppName.substr(start);
}

// Returns base name of the EXE file that launched current process.
std::string GetAppName()
{
    const std::string& sAppName = GetModuleName(NULL);
    size_t end = sAppName.find_last_of('.');
    return sAppName.substr(0, end);
}


// Formats a string of file size
std::string FileSizeToStr(ULONG64 uFileSize)
{
    char szSize[MAX_PATH];
    if (uFileSize == 0)
    {
        return "0 KB";
    }
    else if (uFileSize < 1024)
    {
        float fSizeKbytes = (float)uFileSize / 1024.0f;
        _snprintf(szSize, MAX_PATH, ("%0.1f KB"), fSizeKbytes);
    }
    else if (uFileSize < 1024*1024)
    {
        _snprintf(szSize, MAX_PATH, ("%I64u KB"), uFileSize/1024);
    }
    else
    {
        float fSizeMbytes = (float)uFileSize / (float)(1024*1024);
        _snprintf(szSize, MAX_PATH, ("%0.1f MB"), fSizeMbytes);
    }
    return szSize;
}

// Description of calling thread's last error code
std::string GetErrorMessage(DWORD dwError)
{
    char szbuffer[BUFSIZ];
    DWORD dwLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        dwError, 0, szbuffer, BUFSIZ-1, NULL);
    if (dwLen == 0)
    {
        return ("?");
    }
    return std::string(szbuffer, dwLen);
}

// Write formatted log text to file
void LogFileF(const char* file, const char* fmt, ...)
{
    assert(file && fmt);
    char buffer[BUFSIZ];
    va_list ap;
    va_start(ap, fmt);
    int count = _vsnprintf(buffer, BUFSIZ, fmt, ap);
    va_end(ap);
    if (count <= 0)
    {
        return ;
    }
    FILE* fp = fopen(file, "a+");
    if (fp)
    {
        fwrite(buffer, sizeof(char), count, fp);
        fclose(fp);
    }
}

void LogFile(const char* file, const char* text, int len)
{
    assert(file && text);
    FILE* fp = fopen(file, "a+");
    if (fp)
    {
        fwrite(text, sizeof(char), len, fp);
        fclose(fp);
    }
}


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


// ctor
DbghlpDll::DbghlpDll()
: DllHandle(("dbghelp.dll"))
{
    if (handle_ && init())
    {
        // turn on default options
        DWORD dwOptions = this->SymGetOptions();
        dwOptions |= SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_DEBUG;
        this->SymSetOptions(dwOptions);
    }
    else
    {
        LOG_LAST_ERROR();
    }
}

// dctor
DbghlpDll::~DbghlpDll()
{
    this->SymCleanup(GetCurrentProcess());
}

BOOL DbghlpDll::init()
{
    SymGetOptions = (SymGetOptions_t)GetFuncAddress(("SymGetOptions"));
    SymSetOptions = (SymSetOptions_t)GetFuncAddress(("SymSetOptions"));
    SymInitialize = (SymInitialize_t)GetFuncAddress(("SymInitialize"));
    SymCleanup = (SymCleanup_t)GetFuncAddress(("SymCleanup"));
    StackWalk = (StackWalk_t)GetFuncAddress(("StackWalk"));
    SymFromAddr = (SymFromAddr_t)GetFuncAddress(("SymFromAddr"));
    SymFunctionTableAccess = (SymFunctionTableAccess_t)GetFuncAddress(("SymFunctionTableAccess"));
    SymGetModuleBase = (SymGetModuleBase_t)GetFuncAddress(("SymGetModuleBase"));
    SymGetLineFromAddr = (SymGetLineFromAddr_t)GetFuncAddress(("SymGetLineFromAddr"));
    SymSetContext = (SymSetContext_t)GetFuncAddress(("SymSetContext"));
    SymEnumSymbols = (SymEnumSymbols_t)GetFuncAddress(("SymEnumSymbols"));
    SymGetTypeInfo = (SymGetTypeInfo_t)GetFuncAddress(("SymGetTypeInfo"));
    EnumerateLoadedModules = (EnumerateLoadedModules_t)GetFuncAddress(("EnumerateLoadedModules"));
    MiniDumpWriteDump = (MiniDumpWriteDump_t)GetFuncAddress(("MiniDumpWriteDump"));

    return (SymGetOptions && SymSetOptions && SymInitialize && SymCleanup
        && StackWalk && SymFromAddr && SymFunctionTableAccess && SymGetModuleBase
        && SymGetLineFromAddr && SymSetContext && SymEnumSymbols
        && SymGetTypeInfo && EnumerateLoadedModules && MiniDumpWriteDump);
}

// DbgHelp dll wrapper object
DbghlpDll&  GetDbghelpDll()
{
    static DbghlpDll   instance;
    return instance;
}

