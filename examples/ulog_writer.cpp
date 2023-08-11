/****************************************************************************
 * Copyright (c) 2023 PX4 Development Team.
 * SPDX-License-Identifier: BSD-3-Clause
 ****************************************************************************/

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <ulog_cpp/simple_writer.hpp>
// #include "escape_structs.hpp"

using namespace std::chrono_literals;

static uint64_t currentTimeUs()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}
#if 1
struct MyData1 {
  uint64_t timestamp;
  float debug_array[4];
  float cpuload;
  float temperature;
  int8_t counter;

  static std::string messageName() { return "Captain_Info"; }

  static std::vector<ulog_cpp::Field> fields()
  {
    // clang-format off
    return {
        {"uint64_t", "timestamp"}, // Monotonic timestamp in microseconds (since boot), must always be the first field
        {"float", "debug_array", 4},
        {"float", "cpuload"},
        {"float", "temperature"},
        {"int8_t", "counter"},
    };  // clang-format on
  }
};

struct MyData2 {
  uint64_t timestamp;
  float debug_array[4];
  float cpuload;
  float temperature;
  int8_t counter;

  static std::string messageName() { return "Captain_Action"; }

  static std::vector<ulog_cpp::Field> fields()
  {
    // clang-format off
    return {
        {"uint64_t", "timestamp"}, // Monotonic timestamp in microseconds (since boot), must always be the first field
        {"float", "debug_array", 4},
        {"float", "cpuload"},
        {"float", "temperature"},
        {"int8_t", "counter"},
    };  // clang-format on
  }
};
#endif
int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("Usage: %s <file.ulg>\n", argv[0]);
    return -1;
  }
  /*
    Initialize Writer {

      writer.writeInfo("sys_name", "ULogExampleWriter");
      writer.writeParameter("PARAM_A", 382.23F); // 非必须
      依赖自定义数据MyData
      writer.writeMessageFormat(MyData::messageName(), MyData::fields());
      writer.headerComplete();

      get msg id
      writer.writeAddLoggedMessage(MyData::messageName());
      writer.writeTextMessage(ulog_cpp::Logging::Level::Info, "Hello world", currentTimeUs()); // 非必须

      后续使用 msg id 可以持续写入填充完整的MyData数据
    }
  */
  try {
    // 创建一个SimpleWriter对象，使用提供的文件路径和当前时间作为日志的起始时间。
    ulog_cpp::SimpleWriter writer(argv[1], currentTimeUs());
    // See https://docs.px4.io/main/en/dev_log/ulog_file_format.html#i-information-message for
    // well-known keys
    // 使用写入器对象将一些信息和参数写入日志
    // writer.writeInfo("sys_name", "ULogExampleWriter");
    writer.writeInfo("Captain", "ULogWriter");

    // 非必须
    writer.writeParameter("PARAM_A", 382.23F);
    writer.writeParameter("PARAM_B", 8272);
    // 可以通过不同的Mydata结构体,去区分不同的任务，进程，线程，等数据
    // MyData1 data1{};
    // MyData2 data2{};
    // writer.writeMessageFormat(data1.messageName(), data1.fields());
    // writer.writeMessageFormat(data2.messageName(), data2.fields());
    printf("%s %ld\n",MyData1::messageName().c_str(), MyData1::fields().size());
    writer.writeMessageFormat(MyData1::messageName(), MyData1::fields());
    writer.writeMessageFormat(MyData2::messageName(), MyData2::fields());
    writer.headerComplete();

    const uint16_t my_data_msg_id = writer.writeAddLoggedMessage(MyData1::messageName());
    const uint16_t my_data_msg_id2 = writer.writeAddLoggedMessage(MyData2::messageName());
    // uint16_t my_data_msg_id = writer.writeAddLoggedMessage(data1.messageName());
    // uint16_t my_data_msg_id2 = writer.writeAddLoggedMessage(data2.messageName());

    printf("MyData1 message id: %u\n", my_data_msg_id);
    printf("MyData2 message id: %u\n", my_data_msg_id2);
    // 非必须
    writer.writeTextMessage(ulog_cpp::Logging::Level::Info, "Hello world", currentTimeUs());
    printf("Data size: %ld\n", sizeof(MyData1));
    float cpuload = 25.423F;
    for (int i = 0; i < 100; ++i) {
      MyData1 data{};
      data.timestamp = currentTimeUs();
      data.cpuload = cpuload;
      data.counter = i;
      for(int j = 0; j < 4; ++j) {
        data.debug_array[j] = i + j;
      }
      writer.writeData(my_data_msg_id, data);
      cpuload -= 0.424F;

      std::this_thread::sleep_for(10ms);
    }

    writer.fsync();

    float cpuload2 = 50.846F;
    for (int i = 0; i < 100; ++i) {
      MyData2 data{};
      data.timestamp = currentTimeUs();
      data.cpuload = cpuload2;
      data.counter = i;
      for(int j = 0; j < 4; ++j) {
        data.debug_array[j] = i + j;
      }
      writer.writeData(my_data_msg_id2, data);
      cpuload2 -= 0.848F;
    }

  } catch (const ulog_cpp::ExceptionBase& e) {
    printf("ULog exception: %s\n", e.what());
  }

  return 0;
}
