#include "platform.h"

#include <pthread.h>

#define THREAD_PROC_RET void*

struct Thread {
    pthread_t handle;
};

inline Thread CreateWorkThread(THREAD_PROC_RET (*startFunc) (void*), void* arguments) {
    pthread_t thread;
    pthread_create(&thread, NULL, startFunc, arguments);
    
    Thread result = {};
    result.handle = thread;

    return result;
}

inline void CloseThreadHandle(Thread thread) {
    pthread_cancel(thread.handle);
}

inline uint64_t InterlockedAddAndReturnPrevious(volatile uint64_t* dest, uint64_t value) {
    return __sync_fetch_and_add(dest, value);
}

inline uint32_t InterlockedAddAndReturnPrevious(volatile uint32_t* dest, uint32_t value) {
    return __sync_fetch_and_add(dest, value);
}
