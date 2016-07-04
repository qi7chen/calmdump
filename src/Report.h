// Copyright (C) 2013-2016 ichenq@outlook.com. All rights reserved.
// Distributed under the terms and conditions of the Apache License.
// See accompanying files LICENSE.

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


static const char* const rgBaseType[] =
{
    " <user defined> ",             // btNoType = 0,
    " void ",                       // btVoid = 1,
    " char* ",                      // btChar = 2,
    " wchar_t* ",                   // btWChar = 3,
    " signed char ",
    " unsigned char ",
    " int ",                        // btInt = 6,
    " unsigned int ",               // btUInt = 7,
    " float ",                      // btFloat = 8,
    " <BCD> ",                      // btBCD = 9,
    " bool ",                       // btBool = 10,
    " short ",
    " unsigned short ",
    " long ",                       // btLong = 13,
    " unsigned long ",              // btULong = 14,
    " __int8 ",
    " __int16 ",
    " __int32 ",
    " __int64 ",
    " __int128 ",
    " unsigned __int8 ",
    " unsigned __int16 ",
    " unsigned __int32 ",
    " unsigned __int64 ",
    " unsigned __int128 ",
    " <currency> ",                 // btCurrency = 25,
    " <date> ",                     // btDate = 26,
    " VARIANT ",                    // btVariant = 27,
    " <complex> ",                  // btComplex = 28,
    " <bit> ",                      // btBit = 29,
    " BSTR ",                       // btBSTR = 30,
    " HRESULT "                     // btHresult = 31
};


// Create stack frame log of this exception
void CreateReport(EXCEPTION_POINTERS* ep);
