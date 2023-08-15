#pragma once
#include <cstdint>
#include <string>

struct MyData1 {
    uint64_t timestamp;
    float debug_array[4];
    float cpuload;
    float temperature;
    int8_t counter;
    int32_t array[6];
};

struct MyData2 {
    uint64_t timestamp;
    float debug_array[4];
    float cpuload;
    float temperature;
    int8_t counter;
};

struct MyData3 {
    uint64_t timestamp;
    float debug_array[4];
    float cpuload;
    float temperature;
    int8_t counter;
};