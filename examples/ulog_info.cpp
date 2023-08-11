/****************************************************************************
 * Copyright (c) 2023 PX4 Development Team.
 * SPDX-License-Identifier: BSD-3-Clause
 ****************************************************************************/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <ulog_cpp/data_container.hpp>
#include <ulog_cpp/reader.hpp>
#include <variant>

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("Usage: %s <file.ulg>\n", argv[0]);
    return -1;
  }
  FILE* file = fopen(argv[1], "rb");
  if (!file) {
    printf("opening file failed\n");
    return -1;
  }
  uint8_t buffer[4048];
  int bytes_read;
  // 创建一个DataContainer对象，用于存储从ULog文件解析的数据。
  const auto data_container =
      std::make_shared<ulog_cpp::DataContainer>(ulog_cpp::DataContainer::StorageConfig::FullLog);
  // 创建一个Reader对象，并从输入文件中读取数据块到缓冲区。
  ulog_cpp::Reader reader{data_container};
  // 将每个数据块传递给Reader进行解析
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    reader.readChunk(buffer, bytes_read);
  }
  fclose(file);
  // 在读取完所有数据后，检查是否有解析错误，并打印错误信息（如果有）。
  // Check for errors
  if (!data_container->parsingErrors().empty()) {
    printf("###### File Parsing Errors ######\n");
    for (const auto& parsing_error : data_container->parsingErrors()) {
      printf("   %s\n", parsing_error.c_str());
    }
  }
  // 检查是否有致命的解析错误，如果有则退出。
  if (data_container->hadFatalError()) {
    printf("Fatal parsing error, exiting\n");
    return -1;
  }

  // Print info
  // Dropouts
  // 打印关于数据中丢包的信息。
  const auto& dropouts = data_container->dropouts();
  const int total_dropouts_ms = std::accumulate(
      dropouts.begin(), dropouts.end(), 0,
      [](int sum, const ulog_cpp::Dropout& curr) { return sum + curr.durationMs(); });
  printf("Dropouts: %zu, total duration: %i ms\n", dropouts.size(), total_dropouts_ms);

  auto print_value = [](const std::string& name, const ulog_cpp::Value& value) {
    if (const auto* const str_ptr(std::get_if<std::string>(&value.data())); str_ptr) {
      printf(" %s: %s\n", name.c_str(), str_ptr->c_str());
    } else if (const auto* const int_ptr(std::get_if<int32_t>(&value.data())); int_ptr) {
      printf(" %s: %i\n", name.c_str(), *int_ptr);
    } else if (const auto* const uint_ptr(std::get_if<uint32_t>(&value.data())); uint_ptr) {
      printf(" %s: %u\n", name.c_str(), *uint_ptr);
    } else if (const auto* const float_ptr(std::get_if<float>(&value.data())); float_ptr) {
      printf(" %s: %.3f\n", name.c_str(), static_cast<double>(*float_ptr));
    } else {
      printf(" %s: <data>\n", name.c_str());
    }
  };
  // 打印关于信息消息、多信息消息和数据中包含的消息的信息。
  // Info messages
  printf("Info Messages:\n");
  for (const auto& info_msg : data_container->messageInfo()) {
    print_value(info_msg.second.field().name, info_msg.second.value());
  }
  // Info multi messages
  printf("Info Multiple Messages:");
  for (const auto& info_msg : data_container->messageInfoMulti()) {
    printf(" [%s: %zu],", info_msg.first.c_str(), info_msg.second.size());
  }
  printf("\n");

  // Messages
  printf("\n");
  printf("Name (multi id)  - number of data points\n");

  // Sort by name & multi id
  const auto& subscriptions = data_container->subscriptions();
  std::vector<uint16_t> sorted_subscription_ids(subscriptions.size());
  std::transform(subscriptions.begin(), subscriptions.end(), sorted_subscription_ids.begin(),
                 [](const auto& pair) { return pair.first; });
  std::sort(sorted_subscription_ids.begin(), sorted_subscription_ids.end(),
            [&subscriptions](const uint16_t a, const uint16_t b) {
              const auto& add_logged_a = subscriptions.at(a).add_logged_message;
              const auto& add_logged_b = subscriptions.at(b).add_logged_message;
              if (add_logged_a.messageName() == add_logged_b.messageName()) {
                return add_logged_a.multiId() < add_logged_b.multiId();
              }
              return add_logged_a.messageName() < add_logged_b.messageName();
            });
  for (const auto& subscription_id : sorted_subscription_ids) {
    const auto& subscription = subscriptions.at(subscription_id);
    const int multi_instance = subscription.add_logged_message.multiId();
    const std::string message_name = subscription.add_logged_message.messageName();
    printf(" %s (%i)   -  %zu\n", message_name.c_str(), multi_instance, subscription.data.size());
  }

// 打印关于消息格式的信息。
  printf("Formats:\n");
  for (const auto& msg_format : data_container->messageFormats()) {
    std::string format_fields;
    for (const auto& field : msg_format.second.fields()) {
      format_fields += field.encode() + ", ";
    }
    printf(" %s: %s\n", msg_format.second.name().c_str(), format_fields.c_str());
  }

// 打印关于日志消息的信息。
  // logging
  printf("Logging:\n");
  for (const auto& logging : data_container->logging()) {
    std::string tag_str;
    if (logging.hasTag()) {
      tag_str = std::to_string(logging.tag()) + " ";
    }
    printf(" %s<%s> %lu %s\n", tag_str.c_str(), logging.logLevelStr().c_str(), logging.timestamp(),
           logging.message().c_str());
  }

// 打印关于默认参数和初始参数的信息。
  // Params (init, after, defaults)
  printf("Default Params:\n");
  for (const auto& default_param : data_container->defaultParameters()) {
    print_value(default_param.second.field().name, default_param.second.value());
  }
  printf("Initial Params:\n");
  for (const auto& default_param : data_container->initialParameters()) {
    print_value(default_param.second.field().name, default_param.second.value());
  }

  return 0;
}
