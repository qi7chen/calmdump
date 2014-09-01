// Copyright 2014 ichenq@gmail.com


#pragma once

#include "Utility.h"

// function types
typedef DWORD(WINAPI* SymGetOptions_t)();
typedef DWORD(WINAPI* SymSetOptions_t)(DWORD);
typedef BOOL(WINAPI* SymInitialize_t)(HANDLE, LPSTR, BOOL);
typedef BOOL(WINAPI* StackWalk_t)(DWORD, HANDLE, HANDLE, LPSTACKFRAME,
                                  LPVOID, PREAD_PROCESS_MEMORY_ROUTINE,
                                  PFUNCTION_TABLE_ACCESS_ROUTINE,
                                  PGET_MODULE_BASE_ROUTINE,
                                  PTRANSLATE_ADDRESS_ROUTINE);
typedef BOOL(WINAPI* SymFromAddr_t)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
typedef LPVOID(WINAPI* SymFunctionTableAccess_t)(HANDLE, DWORD_PTR);
typedef DWORD_PTR(WINAPI* SymGetModuleBase_t)(HANDLE, DWORD_PTR);
typedef BOOL(WINAPI* SymGetLineFromAddr_t)(HANDLE, DWORD_PTR,
                                           PDWORD, PIMAGEHLP_LINE);
typedef BOOL(WINAPI* SymSetContext_t)(HANDLE, PIMAGEHLP_STACK_FRAME, PIMAGEHLP_CONTEXT);
typedef BOOL(WINAPI* SymEnumSymbols_t)(HANDLE, ULONG64, PCSTR, PSYM_ENUMERATESYMBOLS_CALLBACK, PVOID);
typedef BOOL(WINAPI* SymGetTypeInfo_t)(HANDLE, DWORD64, ULONG, IMAGEHLP_SYMBOL_TYPE_INFO, PVOID);
typedef BOOL(WINAPI* SymCleanup_t)(HANDLE);
typedef BOOL(WINAPI* EnumerateLoadedModules_t)(HANDLE, PENUMLOADED_MODULES_CALLBACK, PVOID);
typedef BOOL(WINAPI* MiniDumpWriteDump_t)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
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
