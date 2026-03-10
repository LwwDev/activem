//sample method 
#include "sampler.h"
#include <sstream>
#include <sys/sysctl.h>
#include <mach/mach.h>









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

Metrics Sampler::sample(){
    uint64_t total = 0;
    size_t size = sizeof(total); 
    sysctlbyname("hw.memsize", &total, &size, nullptr, 0);
    double totalMB = total/ 1024 / 1024;

    vm_size_t pageSize;
    vm_statistics64_data_t vmStats;
    mach_msg_type_number_t count = sizeof(vmStats) / sizeof(natural_t);
    host_page_size(mach_host_self(), &pageSize);
    host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmStats, &count);


    double availMB = (vmStats.free_count * pageSize) / 1024/1024;
    double usedMB = totalMB - availMB;

    return Metrics{ 42.0, usedMB, totalMB, 0};
}