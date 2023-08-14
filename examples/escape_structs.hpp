#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <ulog_cpp/zz_data_log.hpp>

struct MyData1 {
    uint64_t timestamp;
    float debug_array[4];
    float cpuload;
    float temperature;
    int8_t counter;
    int32_t array[6];

    static std::string messageName()  { return "MyData1"; }

    static std::vector<ulog_cpp::Field> fields()  {
        // clang-format off
        return {
            {"uint64_t", "timestamp"},
            {"float", "debug_array", 4},
            {"float", "cpuload"},
            {"float", "temperature"},
            {"int32_t", "array", 6},
            {"int8_t", "counter"},
        };
        // clang-format on
    }
};

struct MyData2 {
    uint64_t timestamp;
    float debug_array[4];
    float cpuload;
    float temperature;
    int8_t counter;

    static std::string messageName()  { return "MyData2"; }

    static std::vector<ulog_cpp::Field> fields()  {
        // clang-format off
        return {
            {"uint64_t", "timestamp"},
            {"float", "debug_array", 4},
            {"float", "cpuload"},
            {"float", "temperature"},
            {"int8_t", "counter"},
        };
        // clang-format on
    }
};

struct MyData3 {
    uint64_t timestamp;
    float debug_array[4];
    float cpuload;
    float temperature;
    int8_t counter;

    static std::string messageName()  { return "MyData3"; }

    static std::vector<ulog_cpp::Field> fields()  {
        // clang-format off
        return {
            {"uint64_t", "timestamp"},
            {"float", "debug_array", 4},
            {"float", "cpuload"},
            {"float", "temperature"},
            {"int8_t", "counter"},
        };
        // clang-format on
    }
};

// Disable editing of DataVariant variable names
using DataVariant = std::variant<MyData1, MyData2, MyData3>;

static std::vector<DataVariant> all_structs = {
    MyData1{},
    MyData2{},
    MyData3{},
};

