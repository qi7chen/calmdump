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
#include <stdarg.h> // For va_list and related operations
#include <Windows.h>
#include <dbghelp.h>
#include <string>

#pragma warning(disable: 4127)

// Formats a string of file size
std::string FileSizeToStr(ULONG64 uFileSize);

// Printf-like, but std::string as return value
std::string StringPrintf(const char* format, ...);

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
void StringAppendV(std::string* dst, const char* format, va_list ap);

// Returns mudule name of the launched current process.
std::string GetModuleName(HMODULE hModule);


// Returns base name of the EXE file that launched current process.
std::string GetAppName();


// Description of calling thread's last error code
std::string GetErrorMessage(DWORD dwError);


// Write text string to file
void WriteTextToFile(const std::string& module, const std::string& message);


// Get exception pointers
void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS* pExceptionPointers);


#define LogLastError()   do { \
                            std::string msg = GetErrorMessage(::GetLastError()); \
                            WriteTextToFile("FATAL", StringPrintf("%s()[Line: %d][Error: 0x%x]\n%s\r\n", \
                                __FUNCTION__, __LINE__, ::GetLastError(), msg.c_str())); \
                         } while (false);


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
            handle_ = NULL;
        }
    }

    FARPROC GetFuncAddress(const char* funcname)
    {
        assert(handle_ && funcname);
        return ::GetProcAddress(handle_, funcname);
    }

    HMODULE     handle_;
};
