//////////////////////////////////////////////////////////////////////////
// @file    DumpReport.h 
// @author  ichenq@gmail.com 
// @date    Jul, 2013
// @brief  
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>

enum
{
    // Max symbol name length
    MAX_NAME_LEN = 1024,

    // To prevent recursion which could result from corrupted data we limit
    // ourselves to that many levels of embedded fields inside structs
    MAX_DUMP_DEPTH = 20,

    // Max buffer length
    MAX_BUF_SIZE = 4 * 1024,
};

// Create stack frame log of this exception
void CreateReport(EXCEPTION_POINTERS* ep);
