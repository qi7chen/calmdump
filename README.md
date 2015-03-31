# calmdump

calmdump is a C++ server-end crash reporting component for Windows.

## Introduction

[calmdump] [1] takes the core exception handling code in [crashrpt] [2],
without screen capturing, file compression, video recording or anything else for client only usage.

[calmdump] [1] aims to be a lightweight component of a C++ server-end software, mostly used for crash
debugging case, when our server app crashes, calmdump collect the minidump file and print stack 
information to a specified file. that's what it do and what it don't do.

## Usage

~~~~~~~~cpp
#include <Windows.h>
#include <CrashRpt.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    crInstall();    // this API handles everything
    
    // do your own job
}
~~~~~~~~

## Installation

1. Obtain premake4(http://industriousone.com/premake/download)
2. Use premake4 to generate Visual Studio project files, e.g. `premake4 vs2008`


## Further Reading

[A Crash Course on the Depths of Win32™ Structured Exception Handling](http://www.microsoft.com/msj/0197/exception/exception.aspx)

[Vectored Exception Handling](http://msdn.microsoft.com/en-us/magazine/cc301714.aspx)

[Programming against the x64 exception handling support](http://www.nynaeve.net/?p=99)

[Structure Exception Handling in Windows NT](http://www.longene.org/techdoc/0031255001224576939.html)  (in Chinese)


[1]: https://github.com/ichenq/calmdump
[2]: http://code.google.com/p/crashrpt "crashrpt"
