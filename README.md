## Introduction

CrashHandler is a lightweight C++ 20 library for Windows that detects mostly common and some rarer but useful crashes caused by the program the crash handler is attached to. It will display crash messages in the error popup and it will generate a log file with extra info and a dmp file for debugging.

![Crash Screenshot](images/crash_popup.png)

## How to use

```cpp
#include <string>
#include <cstdlib> //for shutting down program

#include "crashHandler.hpp"

using ElypsoUtils::CrashHandler;

static void Shutdown()
{
    exit(1); //non-zero - error
}

int main()
{
    //call this function to initialize crash handler
    CrashHandler::Initialize();

    //attach a string to this function to set the name of the program that will be displayed when the program crashes
    std::string name = "MyProgramName";
    CrashHandler::SetProgramName(name);

    //attach any function similarly to this that will be called
    //once the user presses Ok or closes the error popup
    CrashHandler::SetShutdownCallback([] { Shutdown(); });

    return 0;
}
```

## Types of crashes that are supported

### Access violation (nullptr or invalid memory access)

    Occurs when a program dereferences a null or invalid pointer.  
    Typically caused by reading from or writing to memory that hasn’t been allocated.  
    Exception: `EXCEPTION_ACCESS_VIOLATION`

### Memory access failed (missing or swapped-out memory page)

    Occurs when accessing a valid memory address whose backing storage (e.g. memory-mapped file or pagefile)  
    could not be loaded into memory — typically due to I/O errors or missing file mappings.  
    Exception: `EXCEPTION_IN_PAGE_ERROR`

### Misaligned memory access for specific data type

    Happens when data (e.g. a double) is accessed from an improperly aligned memory address.  
    On some CPUs (e.g. ARM), this causes a crash due to alignment requirements.  
    Exception: `EXCEPTION_DATATYPE_MISALIGNMENT`

### Array bounds exceeded

    Reading or writing outside the bounds of an array, leading to unpredictable behavior or crashes.  
    Often results in an access violation on systems that detect it.  
    Exception: `EXCEPTION_ARRAY_BOUNDS_EXCEEDED`

### Guard page accessed (likely stack guard or memory protection violation)

    Accessing a guard page (used for stack expansion or protected memory regions)  
    triggers a controlled crash, often seen just before a stack overflow.  
    Exception: `EXCEPTION_GUARD_PAGE`

### Integer overflow with overflow checking enabled

    Occurs when signed integer arithmetic exceeds the representable range,  
    and overflow checking is enabled (e.g. with `/RTC` or `_overflow` intrinsics).  
    Exception: `EXCEPTION_INT_OVERFLOW`

### Integer divide by zero

    Dividing an integer by zero results in a structured exception.  
    This is a definite crash on Windows platforms.  
    Exception: `EXCEPTION_INT_DIVIDE_BY_ZERO`

### Privileged instruction executed in user mode

    Executing a privileged CPU instruction (e.g. `hlt`, `cli`) from user mode causes a crash.  
    These are restricted to kernel-mode code.  
    Exception: `EXCEPTION_PRIV_INSTRUCTION`

### Illegal CPU instruction executed

    Manually executes invalid or undefined CPU instructions (e.g. 0xFF 0xFF 0xFF 0xFF).  
    Always causes an illegal instruction exception.  
    Exception: `EXCEPTION_ILLEGAL_INSTRUCTION`

### Stack overflow (likely due to infinite recursion)

    Triggers infinite recursion or excessive stack allocation until the call stack limit is exceeded.  
    Results in a stack overflow exception and immediate crash.  
    Exception: `EXCEPTION_STACK_OVERFLOW`
