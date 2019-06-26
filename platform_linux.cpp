#include <unistd.h>
#include <time.h>

inline uint64_t GetTimeMilliseconds() {
    timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    uint64_t timeMillis = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    return timeMillis;
}

inline uint32_t GetNumberOfProcessors() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}