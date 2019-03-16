#include <mach/mach_time.h>
#include <unistd.h>

inline uint64_t GetTimeMilliseconds() {
    // Get curret time
    uint64_t time = mach_absolute_time();

    // TODO: This operation may not be success. Check return value.
    mach_timebase_info_data_t tb;
    mach_timebase_info (&tb);

    // Convert time to nanoSeconds
    uint64_t timeNano = time * tb.numer / tb.denom; 

    // Convert to millis
    uint64_t timeMillis = timeNano / 1000000;
    return timeMillis;
}

inline uint32_t GetNumberOfProcessors() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}
