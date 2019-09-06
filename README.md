# calmdump

calmdump是用于Windows C++服务端的crash report组件，从[crashrpt](http://crashrpt.sourceforge.net/)里面提取出核心的异常处理代码，并增加了异常堆栈打印。

[calmdump](https://github.com/ichenq/calmdump) takes the core exception handling code in [crashrpt](http://crashrpt.sourceforge.net/), but no client-only feature(screen capturing, file compression, video recording etc).
on collect the minidump file and print stack information to a specified file. 



## API

```cpp
#include <Windows.h>
#include <CrashRpt.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow)
{
    crInstall();    // this API handles everything

    // do your own job
}
```


## 如何构建本项目

* 支持[CMake](https://cmake.org/download/)和[premake](https://premake.github.io/download.html#v5)构建
* CMake方式 `mkdir build && cd build && cmake ..`
* premake方式 `premake5 vs2013` 或 `premake5 gmake`


## 扩展阅读

[A Crash Course on the Depths of Win32™ Structured Exception Handling](http://www.microsoft.com/msj/0197/exception/exception.aspx)

[Vectored Exception Handling](http://msdn.microsoft.com/en-us/magazine/cc301714.aspx)

[Programming against the x64 exception handling support](http://www.nynaeve.net/?p=99)

[Structure Exception Handling in Windows NT](http://www.longene.org/techdoc/0031255001224576939.html)

