// This code is in the public domain -- castano@gmail.com

// Never include this from a header file.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define VC_EXTRALEAN
//#define _WIN32_WINNT 0x0400 // for SwitchToThread, TryEnterCriticalSection
#include <windows.h>
//#include <process.h> // for _beginthreadex