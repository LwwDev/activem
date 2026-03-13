#include "sampler.h"

#include <sstream>
#include <chrono>

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/sysctl.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

std::string Metrics::toJson() const {
    std::ostringstream o;
    o << "{";
    o << "\"cpu\":" << cpu << ",";
    o << "\"memory used\":" << memUsedMB << ",";
    o << "\"memory total\":" << memTotalMB << ",";
    o << "\"ts\":" << ts;
    o << "}";

    return o.str();
}

Metrics Sampler::sample() {
    double totalMB = 0.0;
    double usedMB = 0.0;
    double cpu = 0.0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryInfo;
    memoryInfo.dwLength = sizeof(memoryInfo);
    if (GlobalMemoryStatusEx(&memoryInfo)) {
        totalMB = static_cast<double>(memoryInfo.ullTotalPhys) / 1024.0 / 1024.0;
        double availMB = static_cast<double>(memoryInfo.ullAvailPhys) / 1024.0 / 1024.0;
        usedMB = totalMB - availMB;
    }

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER idleNow;
        ULARGE_INTEGER kernelNow;
        ULARGE_INTEGER userNow;

        idleNow.LowPart = idleTime.dwLowDateTime;
        idleNow.HighPart = idleTime.dwHighDateTime;
        kernelNow.LowPart = kernelTime.dwLowDateTime;
        kernelNow.HighPart = kernelTime.dwHighDateTime;
        userNow.LowPart = userTime.dwLowDateTime;
        userNow.HighPart = userTime.dwHighDateTime;

        uint64_t idle = idleNow.QuadPart;
        uint64_t total = kernelNow.QuadPart + userNow.QuadPart;

        if (prevTotal_ != 0) {
            uint64_t deltaIdle = idle - prevIdle_;
            uint64_t deltaTotal = total - prevTotal_;

            if (deltaTotal != 0) {
                cpu = (1.0 - static_cast<double>(deltaIdle) / static_cast<double>(deltaTotal)) * 100.0;
            }
        }

        prevIdle_ = idle;
        prevTotal_ = total;
    }
#elif __APPLE__
    uint64_t ramTotal = 0;
    size_t size = sizeof(ramTotal);
    sysctlbyname("hw.memsize", &ramTotal, &size, nullptr, 0);
    totalMB = static_cast<double>(ramTotal) / 1024.0 / 1024.0;

    vm_size_t pageSize;
    vm_statistics64_data_t vmStats;
    mach_msg_type_number_t count = sizeof(vmStats) / sizeof(natural_t);
    host_page_size(mach_host_self(), &pageSize);
    host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmStats, &count);

    double availMB = static_cast<double>(vmStats.free_count * pageSize) / 1024.0 / 1024.0;
    usedMB = totalMB - availMB;

    processor_info_array_t cpuInfo;
    mach_msg_type_number_t cpuInfoCount;
    natural_t cpuCount;
    host_processor_info(
        mach_host_self(),
        PROCESSOR_CPU_LOAD_INFO,
        &cpuCount,
        &cpuInfo,
        &cpuInfoCount
    );

    uint64_t idle = 0;
    uint64_t total = 0;
    for (natural_t i = 0; i < cpuCount; i++) {
        idle += cpuInfo[i * CPU_STATE_MAX + CPU_STATE_IDLE];
        total += cpuInfo[i * CPU_STATE_MAX + CPU_STATE_USER];
        total += cpuInfo[i * CPU_STATE_MAX + CPU_STATE_SYSTEM];
        total += cpuInfo[i * CPU_STATE_MAX + CPU_STATE_IDLE];
        total += cpuInfo[i * CPU_STATE_MAX + CPU_STATE_NICE];
    }

    if (prevTotal_ != 0) {
        uint64_t deltaIdle = idle - prevIdle_;
        uint64_t deltaTotal = total - prevTotal_;

        if (deltaTotal != 0) {
            cpu = (1.0 - static_cast<double>(deltaIdle) / static_cast<double>(deltaTotal)) * 100.0;
        }
    }

    prevIdle_ = idle;
    prevTotal_ = total;
#endif

    auto now = std::chrono::system_clock::now().time_since_epoch();
    int64_t ts = std::chrono::duration_cast<std::chrono::seconds>(now).count();

    return Metrics{cpu, usedMB, totalMB, ts};
}
