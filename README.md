# calmdump

calmdump是用于Windows C++服务端的crash report组件，从[crashrpt] [1]里面提取出核心的异常处理代码，并增加了异常堆栈打印。


## API

~~~~~~~~cpp
#include <Windows.h>
#include <CrashRpt.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    crInstall();    // this API handles everything

    // do your own job
}
~~~~~~~~


## 安装

1. Obtain premake5(http://premake.github.io/download.html)
2. Type `premake5 vs2013` to generate Visual C++ solution files


## 扩展阅读

[A Crash Course on the Depths of Win32™ Structured Exception Handling](http://www.microsoft.com/msj/0197/exception/exception.aspx)

[Vectored Exception Handling](http://msdn.microsoft.com/en-us/magazine/cc301714.aspx)

[Programming against the x64 exception handling support](http://www.nynaeve.net/?p=99)

[Structure Exception Handling in Windows NT](http://www.longene.org/techdoc/0031255001224576939.html)


[1]: http://crashrpt.sourceforge.net/ "crashrpt"
