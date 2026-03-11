// metrics struct and sampler class
#pragma once 
#include <string>
#include <cstdint>

struct Metrics {
    double cpu;
    double memUsedMB;
    double memTotalMB;
    int64_t ts;

    std::string toJson() const;
};

class Sampler {
    public:
        Metrics sample();
    

    private:
        uint64_t prevIdle_ = 0;
        uint64_t prevTotal_ = 0;
};