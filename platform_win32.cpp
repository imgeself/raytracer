#ifndef _PLATFORM_WIN32_CPP_
#define _PLATFORM_WIN32_CPP_

#include "platform.h"

#include <windows.h>

//#define THREAD_PROC_RET DWORD

struct Thread {
    HANDLE handle;
};

inline Thread CreateWorkThread(THREAD_PROC_RET (*startFunc) (void*), void* arguments) {
    HANDLE threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) startFunc, arguments, 0, NULL);

    Thread result = {};
    result.handle = threadHandle;

    return result;
}

inline void CloseThreadHandle(Thread thread) {
    CloseHandle(thread.handle);
}

inline uint64_t InterlockedAddAndReturnPrevious(volatile uint64_t* dest, uint64_t value) {
    return InterlockedExchangeAdd(dest, value);
}

inline uint32_t InterlockedAddAndReturnPrevious(volatile uint32_t* dest, uint32_t value) {
    return InterlockedExchangeAdd(dest, value);
}

inline uint64_t GetTimeMilliseconds() {
    // TODO: This operation may not be success. Check return value.
    LARGE_INTEGER time, frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time);

    time.QuadPart *= 1000;
    time.QuadPart /= frequency.QuadPart;

    // Convert to millis
    return (uint64_t) time.QuadPart;
}

inline uint32_t GetNumberOfProcessors() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    return systemInfo.dwNumberOfProcessors;
}

#endif