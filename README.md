# calmdump

calmdump is a C++ server-end crash reporting component for Windows.

## Introduction

calmdump takes the core exception handling code in [crashrpt] [1],
without screen capturing, file compression or video recording or anything used for client reporting.

calmdump aims to be a lightweight component of a C++ server-end software, mostly used for crash
debugging case, when our server app crashes, calmdump collect the minidump file and print stack 
information to a specified file. that's what it do and what it don't do.


## Installation

1. Obtain and install premake building script tool(http://industriousone.com/premake/download)
2. Use premake4 to generate your version of Visual Studio projects, example: `premake4 vs2008`
3. Double click the Visual Studio solution file to launch Visual Studio IDE.
4. Click `build solution` to build this library.

[1] http://code.google.com/p/crashrpt "crashrpt"