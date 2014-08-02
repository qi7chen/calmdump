/************************************************************************************* 
This file is a part of CrashRpt library.
Copyright (c) 2003-2013 The CrashRpt project authors. All Rights Reserved.

Use of this source code is governed by a BSD-style license
that can be found in the License.txt file in the root of the source
tree. All contributing project authors may
be found in the Authors.txt file in the root of the source tree.
***************************************************************************************/

// File: Utility.h
// Description: Miscellaneous helper functions
// Authors: mikecarruth, zexspectrum
// Date: 


#pragma once

#include <assert.h>
#include <Windows.h>
#include <dbghelp.h>
#include <string>


#ifndef _countof
    template <typename T, unsigned N>
    char (&_countof_helper(const T (&array)[N]))[N];
#define _countof(array) sizeof(_countof_helper(array))
#endif

#define LAST_ERROR_MSG      (GetErrorMessage(::GetLastError()).c_str())

#define LOG_LAST_ERROR()    LogModuleFile("ERROR",                          \
                                ("%s()[Line: %d][Error: 0x%x]\n%s\r\n"),    \
                                __FUNCTION__,                               \
                                __LINE__,                                   \
                                ::GetLastError(),                           \
                                LAST_ERROR_MSG)


// Formats a string of file size
std::string FileSizeToStr(ULONG64 uFileSize);


// Returns mudule name of the launched current process.
std::string GetModuleName(HMODULE hModule);


// Returns base name of the EXE file that launched current process.
std::string GetAppName();


// Description of calling thread's last error code
std::string GetErrorMessage(DWORD dwError);


// Write formatted log text to file
void LogModuleFile(const char* module, const char* fmt, ...);


// Get exception pointers
void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS* pExceptionPointers);


// Wrapper for whatever critical section we have
class CritcalSection
{
public:
    CritcalSection() 
    {
        ::InitializeCriticalSectionAndSpinCount(&m_CritSec, 1024);
    };

    ~CritcalSection() 
    {
        ::DeleteCriticalSection(&m_CritSec);
    }

    void Lock() 
    {
        ::EnterCriticalSection(&m_CritSec);
    };

    void Unlock() 
    {
        ::LeaveCriticalSection(&m_CritSec);
    };

private:
    // noncopyable
    CritcalSection(const CritcalSection& );
    CritcalSection &operator=(const CritcalSection& );

    CRITICAL_SECTION m_CritSec;
};

// Auto free dll handle wrapper
struct DllHandle
{
    explicit DllHandle(const char* path)
        : handle_(::LoadLibraryA(path))
    {
        assert(handle_);
    }

    ~DllHandle()
    {
        if (handle_ != NULL)
        {
            ::FreeLibrary(handle_);
        }
    }

    FARPROC GetFuncAddress(const char* funcname)
    {
        assert(handle_ && funcname);
        return ::GetProcAddress(handle_, funcname);
    }

    HMODULE     handle_;
};
