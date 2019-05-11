#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdint.h>

#ifdef PLATFORM_WIN32
#include <windows.h>
#define THREAD_PROC_RET DWORD
#else
#define THREAD_PROC_RET void*
#endif

// Performance counters
inline uint64_t GetTimeMilliseconds();

// Threading
inline uint32_t GetNumberOfProcessors();

struct Thread;

inline Thread CreateWorkThread(THREAD_PROC_RET (*startFunc) (LPVOID), void* arguments);
inline void CloseThreadHandle(Thread thread);

// Atomics
inline uint64_t InterlockedAddAndReturnPrevious(volatile uint64_t* dest, uint64_t value);
inline uint32_t InterlockedAddAndReturnPrevious(volatile uint32_t* dest, uint32_t value);

#ifdef PLATFORM_WIN32
#include "platform_win32.cpp"
#else
#include "platform_posix.cpp"
#endif

#ifdef PLATFORM_MACOS
#include "platform_mac.cpp"
#elif PLATFORM_WIN32
#include "platform_win32.cpp"
#else
#error "Current platform functions not implemented"
#endif

#endif
