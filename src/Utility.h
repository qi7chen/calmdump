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

//////////////////////////////////////////////////////////////////////////
// @file    Utility.h 
// @author  ichenq@gmail.com 
// @date    Jul, 2013
// @brief  
//////////////////////////////////////////////////////////////////////////


#ifndef _UTILITY_H_
#define _UTILITY_H_

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

#define LOG_LAST_ERROR()    LogFileF("error.log", ("%s()[Line: %d][Error: 0x%x]\n%s\r\n"), \
    __FUNCTION__, __LINE__, ::GetLastError(), LAST_ERROR_MSG)


// Formats a string of file size
std::string FileSizeToStr(ULONG64 uFileSize);


// Returns mudule name of the launched current process.
std::string GetModuleName(HMODULE hModule);


// Returns base name of the EXE file that launched current process.
std::string GetAppName();


// Description of calling thread's last error code
std::string GetErrorMessage(DWORD dwError);


// Write formatted log text to file
void LogFileF(const char* file, const char* fmt, ...);

void LogFile(const char* file, const char* text, int len);


// Get exception pointers
void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS* pExceptionPointers);


// Wrapper for whatever critical section we have
class CritcalSection
{
public:
    CritcalSection() 
    {
        ::InitializeCriticalSection(&m_CritSec);
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

// function types
typedef DWORD (WINAPI* SymGetOptions_t)();
typedef DWORD (WINAPI* SymSetOptions_t)(DWORD);
typedef BOOL (WINAPI* SymInitialize_t)(HANDLE, LPSTR, BOOL);
typedef BOOL (WINAPI* StackWalk_t)(DWORD, HANDLE, HANDLE, LPSTACKFRAME,
                                   LPVOID, PREAD_PROCESS_MEMORY_ROUTINE,
                                   PFUNCTION_TABLE_ACCESS_ROUTINE,
                                   PGET_MODULE_BASE_ROUTINE,
                                   PTRANSLATE_ADDRESS_ROUTINE);
typedef BOOL (WINAPI* SymFromAddr_t)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
typedef LPVOID (WINAPI* SymFunctionTableAccess_t)(HANDLE, DWORD_PTR);
typedef DWORD_PTR (WINAPI* SymGetModuleBase_t)(HANDLE, DWORD_PTR);
typedef BOOL (WINAPI* SymGetLineFromAddr_t)(HANDLE, DWORD_PTR,
                                            PDWORD, PIMAGEHLP_LINE);
typedef BOOL (WINAPI* SymSetContext_t)(HANDLE, PIMAGEHLP_STACK_FRAME,
                                       PIMAGEHLP_CONTEXT);
typedef BOOL (WINAPI* SymEnumSymbols_t)(HANDLE, ULONG64, PCSTR,
                                        PSYM_ENUMERATESYMBOLS_CALLBACK, PVOID);
typedef BOOL (WINAPI* SymGetTypeInfo_t)(HANDLE, DWORD64, ULONG,
                                        IMAGEHLP_SYMBOL_TYPE_INFO, PVOID);
typedef BOOL (WINAPI* SymCleanup_t)(HANDLE);
typedef BOOL (WINAPI* EnumerateLoadedModules_t)(HANDLE, PENUMLOADED_MODULES_CALLBACK, PVOID);
typedef BOOL (WINAPI* MiniDumpWriteDump_t)(HANDLE, DWORD, HANDLE,
                                           MINIDUMP_TYPE,
                                           CONST PMINIDUMP_EXCEPTION_INFORMATION,
                                           CONST PMINIDUMP_USER_STREAM_INFORMATION,
                                           CONST PMINIDUMP_CALLBACK_INFORMATION);
// wrapper class for dbghelp.dll
struct DbghlpDll : public DllHandle
{
    DbghlpDll();
    ~DbghlpDll();

    BOOL    init();

    SymGetOptions_t             SymGetOptions;
    SymSetOptions_t             SymSetOptions;
    SymInitialize_t             SymInitialize;
    SymCleanup_t                SymCleanup;
    StackWalk_t                 StackWalk;
    SymFromAddr_t               SymFromAddr;
    SymFunctionTableAccess_t    SymFunctionTableAccess;
    SymGetModuleBase_t          SymGetModuleBase;
    SymGetLineFromAddr_t        SymGetLineFromAddr;
    SymSetContext_t             SymSetContext;
    SymEnumSymbols_t            SymEnumSymbols;
    SymGetTypeInfo_t            SymGetTypeInfo;
    EnumerateLoadedModules_t    EnumerateLoadedModules;
    MiniDumpWriteDump_t         MiniDumpWriteDump;
};


// DbgHelp dll wrapper object
DbghlpDll&  GetDbghelpDll();


#endif	// _UTILITY_H_
