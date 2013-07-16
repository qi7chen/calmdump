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

#include <Windows.h>
#include <string>

#ifndef _countof
    template <typename T, unsigned N>
    char (&_countof_helper(const T (&array)[N]))[N];
#define _countof(array) sizeof(_countof_helper(array))
#endif

// wrapper for whatever critical section we have
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

// Formats a string of file size
std::string FileSizeToStr(ULONG64 uFileSize);

// Returns base name of the EXE file that launched current process.
std::string GetAppName();

// Suspend all other threads to freeze the current state
void SuspendOtherThreads();

// Get exception pointers
void GetExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS* pExceptionPointers);


#endif	// _UTILITY_H_
