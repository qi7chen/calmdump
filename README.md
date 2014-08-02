# calmdump

calmdump is a C++ server-end crash reporting component for Windows.

## Introduction

[calmdump] [1] takes the core exception handling code in [crashrpt] [2],
without screen capturing, file compression or video recording or anything used for client reporting.

[calmdump] [1] aims to be a lightweight component of a C++ server-end software, mostly used for crash
debugging case, when our server app crashes, calmdump collect the minidump file and print stack 
information to a specified file. that's what it do and what it don't do.


## Installation

1. Obtain and install premake building script tool(http://industriousone.com/premake/download)
2. Use premake4 to generate your version of Visual Studio projects, example: `premake4 vs2008`
3. Double click the Visual Studio solution file to launch Visual Studio IDE.
4. Click `build solution` to build this library.


## Further Reading

[A Crash Course on the Depths of Win32™ Structured Exception Handling](http://www.microsoft.com/msj/0197/exception/exception.aspx)

[Vectored Exception Handling](http://msdn.microsoft.com/en-us/magazine/cc301714.aspx)

[Programming against the x64 exception handling support](http://www.nynaeve.net/?p=99)

[Structure Exception Handling in Windows NT](http://www.longene.org/techdoc/0031255001224576939.html)  (in Chinese)


[1]: https://github.com/ichenq/calmdump
[2]: http://code.google.com/p/crashrpt "crashrpt"
