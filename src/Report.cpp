// Copyright 2014 ichenq@gmail.com

#include "Report.h"
#include "Dbghlp.h"
#include "cvconst.h"
#include <time.h>
#include <string>
#include <list>

#pragma warning(disable: 4996)

// Print system information
static void PrintSystemInfo();

// Callback pro to numerate symbols
static BOOL CALLBACK EnumSymbolsProcCallback(PSYMBOL_INFO pSymInfo, ULONG SymSize, PVOID data);

static bool FormatSymbolValue(PSYMBOL_INFO pSym, STACKFRAME* sf, char* pszBuffer, unsigned cbBuffer);

static char* FormatOutputValue(char* pszCurrBuffer, BasicType basicType, DWORD64 length, PVOID pAddress);

static BasicType GetBasicType(DWORD typeIndex, DWORD64 modBase);

static char* DumpTypeIndex(char* pszCurrBuffer, DWORD64 modBase, DWORD dwTypeIndex,
    unsigned nestingLevel, DWORD_PTR offset, bool & bHandled, char* Name);


// Add log text to file
static void AddToReport(const char* fmt, ...)
{
    char buf[MAX_BUF_SIZE];
    va_list ap;
    va_start(ap, fmt);
    int count = _vsnprintf(buf, MAX_BUF_SIZE, fmt, ap);
    va_end(ap);
    if (count <= 0)
    {
        return ;
    }
    std::string name = GetAppName();
    LogModuleFile(name.c_str(), "%s", buf);
}


// Given an exception code, returns a pointer to a static string with a 
// description of the exception                                         
const char* GetExceptionString( DWORD dwCode )
{
#define EXCEPTION( x ) case EXCEPTION_##x: return (#x);
    switch ( dwCode )
    {
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.

    static TCHAR szBuffer[512] = { 0 };
    FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        GetModuleHandle( ("NTDLL.DLL") ),
        dwCode, 0, szBuffer, sizeof( szBuffer ), 0 );

    return szBuffer;
}

static void DumpSymbolName(DWORD dwLevel, const STACKFRAME& sf)
{
    DWORD dwAddress = sf.AddrPC.Offset;
    BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_NAME_LEN] = {};
    PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)symbolBuffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_NAME_LEN;
    DWORD64 symDisplacement = 0;
    if (!GetDbghelpDll().SymFromAddr(::GetCurrentProcess(), dwAddress,
                                     &symDisplacement, pSymbol))
    {
        //LOG_LAST_ERROR();
        return ;
    }

    IMAGEHLP_LINE line = {sizeof(IMAGEHLP_LINE)};
    DWORD dwLineDisplacement = 0;
    if (!GetDbghelpDll().SymGetLineFromAddr(
                ::GetCurrentProcess(),
                dwAddress,
                &dwLineDisplacement,
                &line))
    {
        // it is normal that we don't have source info for some symbols,
        // notably all the ones from the system DLLs...
        return;
    }

    AddToReport(("%02d. (0x%08I64x) %s()  %s [%u]\n"), dwLevel, pSymbol->Address,
                pSymbol->Name, line.FileName, line.LineNumber);
}


static void DumpSymbolParam(const STACKFRAME& sf )
{
    DWORD dwSymAddr = sf.AddrPC.Offset;
    DWORD dwAddrFrame = sf.AddrFrame.Offset;

    // use SymSetContext to get just the locals/params for this frame
    IMAGEHLP_STACK_FRAME imagehlpStackFrame = {};
    imagehlpStackFrame.InstructionOffset = dwSymAddr;
    if (!GetDbghelpDll().SymSetContext(::GetCurrentProcess(), &imagehlpStackFrame, 0))
    {
        // for symbols from kernel DLL we might not have access to their
        // address, this is not a real error
        return ;
    }

    if (!GetDbghelpDll().SymEnumSymbols(
                ::GetCurrentProcess(),
                NULL,                   // DLL base: use current context
                NULL,                   // no mask, get all symbols
                EnumSymbolsProcCallback,
                (PVOID)&sf))    // data parameter for this callback
    {
        LOG_LAST_ERROR();
    }
}

static void WalkStack(const CONTEXT* pContext, size_t skip, size_t maxDepth)
{
    CONTEXT ctx = *pContext; // will be modified by dbghelp StackWalk()
    DWORD dwMachineType;
    // initialize the initial frame: currently we can do it for x86 only
    STACKFRAME sf = {};
#if defined(_M_AMD64)
    sf.AddrPC.Offset       = ctx.Rip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = ctx.Rsp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = ctx.Rbp;
    sf.AddrFrame.Mode      = AddrModeFlat;

    dwMachineType = IMAGE_FILE_MACHINE_AMD64;
#elif  defined(_M_IX86)
    sf.AddrPC.Offset       = ctx.Eip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = ctx.Esp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = ctx.Ebp;
    sf.AddrFrame.Mode      = AddrModeFlat;

    dwMachineType = IMAGE_FILE_MACHINE_I386;
#else
#error "Need to initialize STACKFRAME on non x86"
#endif // _M_IX86

    // iterate over all stack frames
    for ( size_t nLevel = 0; nLevel < maxDepth; nLevel++)
    {
        if (!GetDbghelpDll().StackWalk(
                    dwMachineType,
                    ::GetCurrentProcess(),
                    ::GetCurrentThread(),
                    &sf,
                    &ctx,
                    NULL,   // read memory function (default)
                    GetDbghelpDll().SymFunctionTableAccess,
                    GetDbghelpDll().SymGetModuleBase,
                    NULL))    // address translator for 16 bit
        {
            break;
        }
        //Invalid frame
        if (!sf.AddrFrame.Offset)
        {
            break;
        }

        // don't show this frame itself in the output
        if (nLevel >= skip)
        {
            DumpSymbolName(nLevel - skip, sf);
            DumpSymbolParam(sf);
            AddToReport(("\n"));
        }
    }
}

static void PrintExceptInfo(EXCEPTION_POINTERS* ep)
{
    AddToReport(("\r\n*** Exception ***\r\n"));
    MEMORY_BASIC_INFORMATION mbi;
    char szModule[MAX_PATH];
    DWORD dwExceptCode = ep->ExceptionRecord->ExceptionCode;
    PVOID ExceptionAddress = ep->ExceptionRecord->ExceptionAddress;
    if (VirtualQuery(ExceptionAddress, &mbi, sizeof(mbi)))
    {
        DWORD hMod = (DWORD)mbi.AllocationBase;
        if (GetModuleFileName((HMODULE)hMod, szModule, MAX_PATH))
        {
            AddToReport(("Module: %s\r\n"), szModule);
        }
    }

    AddToReport(("Fault address: 0x%08x, Thread ID: %d\r\n"), ExceptionAddress, GetCurrentThreadId());
    if (dwExceptCode == EXCEPTION_ACCESS_VIOLATION)
    {
        ULONG_PTR* exceptinfo = ep->ExceptionRecord->ExceptionInformation;
        const char* szOperation = (exceptinfo[0] ? ("write") : ("read"));
        AddToReport(("Failed to %d address 0x%08x\r\n"), szOperation, exceptinfo[1]);
    }
    AddToReport(("Exception code: 0x%08x %s\r\n\r\n"), dwExceptCode, GetExceptionString(dwExceptCode));
}

// Create stack frame log of this exception
void CreateReport(EXCEPTION_POINTERS* ep)
{
    assert(ep);

    time_t now = time(NULL);
    AddToReport(("\nException report created at %s"), ctime(&now));

    PrintExceptInfo(ep);    

    // according to MSDN, the first parameter should be just a unique value and
    // not process handle (although the parameter is prototyped as "HANDLE
    // hProcess") and actually it advises to use the process id and not handle
    // for Win9x, but then we need to use the same value in StackWalk() call
    // below which should be a real handle... so this is what we use
    const HANDLE hProcess = ::GetCurrentProcess();
    if (!GetDbghelpDll().SymInitialize(hProcess, NULL, TRUE))
    {
        LOG_LAST_ERROR();
        return ;
    }

    AddToReport(("\r\nCall stack:\r\n---------------------------\r\n"));
    AddToReport(("Level   Address   Function	    SourceFile\r\n"));

    // enumerate stack frames from the given context
    WalkStack(ep->ContextRecord, 0, MAX_DUMP_DEPTH);

    PrintSystemInfo();

    if (!GetDbghelpDll().SymCleanup(::GetCurrentProcess()))
    {
        LOG_LAST_ERROR();
    }
}


BOOL CALLBACK EnumSymbolsProcCallback(PSYMBOL_INFO pSymInfo, ULONG SymSize, PVOID userContext)
{
    STACKFRAME* sf = (STACKFRAME*)userContext;

    // we're only interested in parameters and local variables
    if ( pSymInfo->Flags & SYMF_PARAMETER || pSymInfo->Flags & SYMF_LOCAL)
    {
        char szBuffer[2048];
        __try
        {
            if (FormatSymbolValue(pSymInfo, sf, szBuffer, _countof(szBuffer)))
            {
                AddToReport("\t%s\r\n", szBuffer);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    // return true to continue enumeration, false would have stopped it
    return TRUE;
}


// Given a SYMBOL_INFO representing a particular variable, displays its
// contents.  If it's a user defined type, display the members and their
// values.
bool FormatSymbolValue(PSYMBOL_INFO pSym, STACKFRAME* sf, char* pszBuffer, unsigned cbBuffer)
{
    char* pszCurrBuffer = pszBuffer;

    // Indicate if the variable is a local or parameter
    if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER)
        pszCurrBuffer += sprintf(pszCurrBuffer, "Parameter ");
    else if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL)
        pszCurrBuffer += sprintf(pszCurrBuffer, "Local ");

    // If it's a function, don't do anything.
    if (pSym->Tag == 5)                                   // SymTagFunction from CVCONST.H from the DIA SDK
        return false;

    DWORD_PTR pVariable = 0;                                // Will point to the variable's data in memory

    if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE)
    {
        // if ( pSym->Register == 8 )   // EBP is the value 8 (in DBGHELP 5.1)
        {                                                   //  This may change!!!
            pVariable = sf->AddrFrame.Offset;
            pVariable += (DWORD_PTR)pSym->Address;
        }
        // else
        //  return false;
    }
    else if (pSym->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER)
    {
        return false;                                       // Don't try to report register variable
    }
    else
    {
        pVariable = (DWORD_PTR)pSym->Address;               // It must be a global variable
    }

    // Determine if the variable is a user defined type (UDT).  IF so, bHandled
    // will return true.
    bool bHandled;
    pszCurrBuffer = DumpTypeIndex(pszCurrBuffer, pSym->ModBase, pSym->TypeIndex,
        0, pVariable, bHandled, pSym->Name);

    if (!bHandled)
    {
        // The symbol wasn't a UDT, so do basic, stupid formatting of the
        // variable.  Based on the size, we're assuming it's a char, WORD, or
        // DWORD.
        BasicType basicType = GetBasicType(pSym->TypeIndex, pSym->ModBase);
        pszCurrBuffer += sprintf(pszCurrBuffer, rgBaseType[basicType]);

        // Emit the variable name
        pszCurrBuffer += sprintf(pszCurrBuffer, "\'%s\'", pSym->Name);

        pszCurrBuffer = FormatOutputValue(pszCurrBuffer, basicType, pSym->Size,
            (PVOID)pVariable);
    }

    return true;
}

char* FormatOutputValue(char* pszCurrBuffer, BasicType basicType, DWORD64 length, PVOID pAddress)
{
    // Format appropriately (assuming it's a 1, 2, or 4 bytes (!!!)
    if (length == 1)
        pszCurrBuffer += sprintf(pszCurrBuffer, " = %X", *(PBYTE)pAddress);
    else if (length == 2)
        pszCurrBuffer += sprintf(pszCurrBuffer, " = %X", *(PWORD)pAddress);
    else if (length == 4)
    {
        if (basicType == btFloat)
        {
            pszCurrBuffer += sprintf(pszCurrBuffer, " = %f", *(PFLOAT)pAddress);
        }
        else if (basicType == btChar)
        {
            if (!IsBadStringPtr(*(PSTR*)pAddress, 32))
            {
                pszCurrBuffer += sprintf(pszCurrBuffer, " = \"%.31s\"",
                    *(PDWORD)pAddress);
            }
            else
                pszCurrBuffer += sprintf(pszCurrBuffer, " = %X",
                *(PDWORD)pAddress);
        }
        else
            pszCurrBuffer += sprintf(pszCurrBuffer, " = %X", *(PDWORD)pAddress);
    }
    else if (length == 8)
    {
        if (basicType == btFloat)
        {
            pszCurrBuffer += sprintf(pszCurrBuffer, " = %lf",
                *(double *)pAddress);
        }
        else
            pszCurrBuffer += sprintf(pszCurrBuffer, " = %I64X",
            *(DWORD64*)pAddress);
    }

    return pszCurrBuffer;
}


// If it's a user defined type (UDT), recurse through its members until we're
// at fundamental types.  When he hit fundamental types, return
// bHandled = false, so that FormatSymbolValue() will format them.
char* DumpTypeIndex(char* pszCurrBuffer, DWORD64 modBase, DWORD dwTypeIndex, 
                    unsigned nestingLevel, DWORD_PTR offset, bool & bHandled, char* Name)
{
    bHandled = false;

    // Get the name of the symbol.  This will either be a Type name (if a UDT),
    // or the structure member name.
    WCHAR * pwszTypeName;
    if (GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, dwTypeIndex, TI_GET_SYMNAME,
        &pwszTypeName))
    {
        pszCurrBuffer += sprintf(pszCurrBuffer, " %ls", pwszTypeName);
        LocalFree(pwszTypeName);
    }

    // Determine how many children this type has.
    DWORD dwChildrenCount = 0;
    GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, dwTypeIndex, TI_GET_CHILDRENCOUNT,
        &dwChildrenCount);

    if (!dwChildrenCount)                                 // If no children, we're done
        return pszCurrBuffer;

    // Prepare to get an array of "TypeIds", representing each of the children.
    // SymGetTypeInfo(TI_FINDCHILDREN) expects more memory than just a
    // TI_FINDCHILDREN_PARAMS struct has.  Use derivation to accomplish this.
    struct FINDCHILDREN : TI_FINDCHILDREN_PARAMS
    {
        ULONG   MoreChildIds[1024];
        FINDCHILDREN(){ Count = sizeof(MoreChildIds) / sizeof(MoreChildIds[0]); }
    } children;

    children.Count = dwChildrenCount;
    children.Start = 0;

    // Get the array of TypeIds, one for each child type
    if (!GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, dwTypeIndex, TI_FINDCHILDREN,
        &children))
    {
        return pszCurrBuffer;
    }

    // Append a line feed
    pszCurrBuffer += sprintf(pszCurrBuffer, "\r\n");

    // Iterate through each of the children
    for (unsigned i = 0; i < dwChildrenCount; i++)
    {
        // Add appropriate indentation level (since this routine is recursive)
        for (unsigned j = 0; j <= nestingLevel + 1; j++)
            pszCurrBuffer += sprintf(pszCurrBuffer, "\t");

        // Recurse for each of the child types
        bool bHandled2;
        BasicType basicType = GetBasicType(children.ChildId[i], modBase);
        pszCurrBuffer += sprintf(pszCurrBuffer, rgBaseType[basicType]);

        pszCurrBuffer = DumpTypeIndex(pszCurrBuffer, modBase,
            children.ChildId[i], nestingLevel + 1,
            offset, bHandled2, ""/*Name */);

        // If the child wasn't a UDT, format it appropriately
        if (!bHandled2)
        {
            // Get the offset of the child member, relative to its parent
            DWORD dwMemberOffset;
            GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, children.ChildId[i],
                TI_GET_OFFSET, &dwMemberOffset);

            // Get the real "TypeId" of the child.  We need this for the
            // SymGetTypeInfo( TI_GET_TYPEID ) call below.
            DWORD typeId;
            GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, children.ChildId[i],
                TI_GET_TYPEID, &typeId);

            // Get the size of the child member
            ULONG64 length;
            GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, typeId, TI_GET_LENGTH, &length);

            // Calculate the address of the member
            DWORD_PTR dwFinalOffset = offset + dwMemberOffset;
            pszCurrBuffer = FormatOutputValue(pszCurrBuffer, basicType,
                length, (PVOID)dwFinalOffset);

            pszCurrBuffer += sprintf(pszCurrBuffer, "\r\n");
        }
    }

    bHandled = true;
    return pszCurrBuffer;
}

BasicType GetBasicType(DWORD typeIndex, DWORD64 modBase)
{
    BasicType basicType;
    if (GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, typeIndex,
        TI_GET_BASETYPE, &basicType))
    {
        return basicType;
    }

    // Get the real "TypeId" of the child.  We need this for the
    // SymGetTypeInfo( TI_GET_TYPEID ) call below.
    DWORD typeId;
    if (GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, typeIndex, TI_GET_TYPEID, &typeId))
    {
        if (GetDbghelpDll().SymGetTypeInfo(GetCurrentProcess(), modBase, typeId, TI_GET_BASETYPE,
            &basicType))
        {
            return basicType;
        }
    }

    return btNoType;
}

bool GetProcessorName(char* sProcessorName, DWORD maxcount)
{
    assert(sProcessorName);
    HKEY hKey;
    LONG lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, ("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
                               0, KEY_QUERY_VALUE, &hKey);
    if (lRet != ERROR_SUCCESS)
    {
        return false;
    }
    char szTmp[2048];
    DWORD cntBytes = sizeof(szTmp);
    lRet = ::RegQueryValueEx(hKey, ("ProcessorNameString"), NULL, NULL,
                             (LPBYTE)szTmp, &cntBytes);
    if (lRet != ERROR_SUCCESS)
    {
        return false;
    }
    ::RegCloseKey(hKey);
    sProcessorName[0] = '\0';
    // Skip spaces
    char* psz = szTmp;
    while (isspace(*psz))
    {
        ++psz;
    }
    strncpy(sProcessorName, psz, maxcount);
    return true;
}

typedef void (WINAPI* PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI* PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

BOOL GetWindowsVersion(char* szVersion, DWORD cntMax)
{
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    PGNSI pGNSI;
    PGPI pGPI;
    BOOL bOsVersionInfoEx;
    DWORD dwType;

    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);

    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI) GetProcAddress(
                GetModuleHandle(TEXT("kernel32.dll")),
                "GetNativeSystemInfo");
    if(NULL != pGNSI)
    {
        pGNSI(&si);
    }
    else
    {
        GetSystemInfo(&si);
    }

    if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId &&
            osvi.dwMajorVersion > 4 )
    {
        strcpy_s(szVersion, cntMax, TEXT("Microsoft "));

        // Test for the specific product.

        if ( osvi.dwMajorVersion == 6 )
        {
            if( osvi.dwMinorVersion == 0 )
            {
                if( osvi.wProductType == VER_NT_WORKSTATION )
                {
                    strcat_s(szVersion, cntMax, TEXT("Windows Vista "));
                }
                else
                {
                    strcat_s(szVersion, cntMax, TEXT("Windows Server 2008 " ));
                }
            }
            else if ( osvi.dwMinorVersion == 1 )
            {
                if( osvi.wProductType == VER_NT_WORKSTATION )
                {
                    strcat_s(szVersion, cntMax, TEXT("Windows 7 "));
                }
                else
                {
                    strcat_s(szVersion, cntMax, TEXT("Windows Server 2008 R2 " ));
                }
            }
            else if ( osvi.dwMinorVersion == 2 )
            {
                if( osvi.wProductType == VER_NT_WORKSTATION )
                {
                    strcat_s(szVersion, cntMax, TEXT("Windows 8 "));
                }
                else
                {
                    strcat_s(szVersion, cntMax, TEXT("Windows Server 2012 " ));
                }
            }

            pGPI = (PGPI) GetProcAddress(
                       GetModuleHandle(TEXT("kernel32.dll")),
                       "GetProductInfo");

            pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
            switch( dwType )
            {
                case PRODUCT_ULTIMATE:
                    strcat_s(szVersion, cntMax, TEXT("Ultimate Edition" ));
                    break;
                case PRODUCT_PROFESSIONAL:
                    strcat_s(szVersion, cntMax, TEXT("Professional" ));
                    break;
                case PRODUCT_HOME_PREMIUM:
                    strcat_s(szVersion, cntMax, TEXT("Home Premium Edition" ));
                    break;
                case PRODUCT_HOME_BASIC:
                    strcat_s(szVersion, cntMax, TEXT("Home Basic Edition" ));
                    break;
                case PRODUCT_ENTERPRISE:
                    strcat_s(szVersion, cntMax, TEXT("Enterprise Edition" ));
                    break;
                case PRODUCT_BUSINESS:
                    strcat_s(szVersion, cntMax, TEXT("Business Edition" ));
                    break;
                case PRODUCT_STARTER:
                    strcat_s(szVersion, cntMax, TEXT("Starter Edition" ));
                    break;
                case PRODUCT_CLUSTER_SERVER:
                    strcat_s(szVersion, cntMax, TEXT("Cluster Server Edition" ));
                    break;
                case PRODUCT_DATACENTER_SERVER:
                    strcat_s(szVersion, cntMax, TEXT("Datacenter Edition" ));
                    break;
                case PRODUCT_DATACENTER_SERVER_CORE:
                    strcat_s(szVersion, cntMax, TEXT("Datacenter Edition (core installation)" ));
                    break;
                case PRODUCT_ENTERPRISE_SERVER:
                    strcat_s(szVersion, cntMax, TEXT("Enterprise Edition" ));
                    break;
                case PRODUCT_ENTERPRISE_SERVER_CORE:
                    strcat_s(szVersion, cntMax, TEXT("Enterprise Edition (core installation)" ));
                    break;
                case PRODUCT_ENTERPRISE_SERVER_IA64:
                    strcat_s(szVersion, cntMax, TEXT("Enterprise Edition for Itanium-based Systems" ));
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER:
                    strcat_s(szVersion, cntMax, TEXT("Small Business Server" ));
                    break;
                case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                    strcat_s(szVersion, cntMax, TEXT("Small Business Server Premium Edition" ));
                    break;
                case PRODUCT_STANDARD_SERVER:
                    strcat_s(szVersion, cntMax, TEXT("Standard Edition" ));
                    break;
                case PRODUCT_STANDARD_SERVER_CORE:
                    strcat_s(szVersion, cntMax, TEXT("Standard Edition (core installation)" ));
                    break;
                case PRODUCT_WEB_SERVER:
                    strcat_s(szVersion, cntMax, TEXT("Web Server Edition" ));
                    break;
            }
        }

        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
        {
            if( GetSystemMetrics(SM_SERVERR2) )
            {
                strcat_s(szVersion, cntMax, TEXT( "Windows Server 2003 R2, "));
            }
            else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
            {
                strcat_s(szVersion, cntMax, TEXT( "Windows Storage Server 2003"));
            }
            else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
            {
                strcat_s(szVersion, cntMax, TEXT( "Windows Home Server"));
            }
            else if( osvi.wProductType == VER_NT_WORKSTATION &&
                     si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            {
                strcat_s(szVersion, cntMax, TEXT( "Windows XP Professional x64 Edition"));
            }
            else
            {
                strcat_s(szVersion, cntMax, TEXT("Windows Server 2003, "));
            }

            // Test for the server type.
            if ( osvi.wProductType != VER_NT_WORKSTATION )
            {
                if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Datacenter Edition for Itanium-based Systems" ));
                    }
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Enterprise Edition for Itanium-based Systems" ));
                    }
                }
                else if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Datacenter x64 Edition" ));
                    }
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Enterprise x64 Edition" ));
                    }
                    else
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Standard x64 Edition" ));
                    }
                }
                else
                {
                    if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Compute Cluster Edition" ));
                    }
                    else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Datacenter Edition" ));
                    }
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Enterprise Edition" ));
                    }
                    else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Web Edition" ));
                    }
                    else
                    {
                        strcat_s(szVersion, cntMax, TEXT( "Standard Edition" ));
                    }
                }
            }
        }
        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
        {
            strcat_s(szVersion, cntMax, TEXT("Windows XP "));
            if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
            {
                strcat_s(szVersion, cntMax, TEXT( "Home Edition" ));
            }
            else
            {
                strcat_s(szVersion, cntMax, TEXT( "Professional" ));
            }
        }

        if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
        {
            strcat_s(szVersion, cntMax, TEXT("Windows 2000 "));

            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
                strcat_s(szVersion, cntMax, TEXT( "Professional" ));
            }
            else
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                {
                    strcat_s(szVersion, cntMax, TEXT( "Datacenter Server" ));
                }
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                {
                    strcat_s(szVersion, cntMax, TEXT( "Advanced Server" ));
                }
                else
                {
                    strcat_s(szVersion, cntMax, TEXT( "Server" ));
                }
            }
        }

        // Include service pack (if any) and build number.
        if( strlen(osvi.szCSDVersion) > 0 )
        {
            strcat_s(szVersion, cntMax, TEXT(" ") );
            strcat_s(szVersion, cntMax, osvi.szCSDVersion);
        }

        TCHAR buf[80];
        sprintf_s( buf, 80, TEXT(" (build %d)"), osvi.dwBuildNumber);
        strcat_s(szVersion, cntMax, buf);

        if ( osvi.dwMajorVersion >= 6 )
        {
            if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
            {
                strcat_s(szVersion, cntMax, TEXT( ", 64-bit" ));
            }
            else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
            {
                strcat_s(szVersion, cntMax, TEXT(", 32-bit"));
            }
        }
        return TRUE;
    }
    else
    {
        strcat_s(szVersion, cntMax, "This application does not support this version of Windows.\n");
        return FALSE;
    }

    return TRUE;
}

void PrintSystemInfo()
{
    SYSTEM_INFO SystemInfo;
    ::GetSystemInfo(&SystemInfo);

    MEMORYSTATUS MemoryStatus;
    MemoryStatus.dwLength = sizeof (MEMORYSTATUS);
    ::GlobalMemoryStatus(&MemoryStatus);

    TCHAR sString[1024];
    AddToReport(("=====================================================\r\n"));
    if (GetProcessorName(sString, _countof(sString)))
    {
        AddToReport(("*** Hardware ***\r\nProcessor: %s\r\nNumber Of Processors: %d\r\n"
            "Physical Memory: %s (Available: %s)\r\nCommit Charge Limit: %s\r\n"),
            sString, SystemInfo.dwNumberOfProcessors, FileSizeToStr(MemoryStatus.dwTotalPhys).c_str(), 
            FileSizeToStr(MemoryStatus.dwAvailPhys).c_str(), FileSizeToStr(MemoryStatus.dwTotalPageFile).c_str());
    }
    else
    {
        AddToReport(("*** Hardware ***\r\nProcessor: <unknown>\r\nNumber Of Processors: %d\r\n"
            "Physical Memory: %s (Available: %s)\r\nCommit Charge Limit: %s\r\n"),
                    SystemInfo.dwNumberOfProcessors, FileSizeToStr(MemoryStatus.dwTotalPhys).c_str(), 
                    FileSizeToStr(MemoryStatus.dwAvailPhys).c_str(), FileSizeToStr(MemoryStatus.dwTotalPageFile).c_str()); 
    }

    if(GetWindowsVersion(sString, _countof(sString)))
    {
        AddToReport(("\r\n*** Operation System ***\r\n%s\r\n"), sString);
    }
    else
    {
        AddToReport(("\r\n*** Operation System:\r\n<unknown>\r\n"));
    }
}
