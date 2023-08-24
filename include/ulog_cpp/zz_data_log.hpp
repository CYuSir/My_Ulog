/****************************************************************************
 * Copyright (c) 2023 PX4 Development Team.
 * SPDX-License-Identifier: BSD-3-Clause
 ****************************************************************************/
#pragma once
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <unordered_map>
#include <vector>

#include "writer.hpp"

using namespace std;
using namespace ulog_cpp;
using namespace std::chrono_literals;

extern bool ZzDataLogOn;

inline uint64_t currentTimeUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

struct StructInfo {
    std::string messageNname;
    std::vector<ulog_cpp::Field> fields;
};
// 存储初始化参数的结构体
struct InitParams {
    std::string file_name;
    std::string key;
    std::string key_value;
    std::vector<StructInfo> all_structs;
};

namespace ulog_cpp {
/**
 * ULog serialization class which checks for integrity and correct calling order.
 * It throws an UsageException() in case of a failed integrity check.
 */
class zz_data_log {
    // Declare CreateInstance as a friend function
    friend void CreateInstance(const std::string& filename, bool ZzDataLogOn = true);
    friend std::shared_ptr<zz_data_log> GetInstance();

   public:
    /**
     * Constructor with a callback for writing data.
     * @param data_write_cb callback for serialized ULog data
     * @param timestamp_us start timestamp [us]
     */
    explicit zz_data_log(DataWriteCB data_write_cb, uint64_t timestamp_us);
    /**
     * Constructor to write to a file.
     * @param filename ULog file to write to (will be overwritten if it exists)
     * @param timestamp_us  start timestamp [us]
     */
    explicit zz_data_log(const std::string& filename, uint64_t timestamp_us);

    explicit zz_data_log(const std::string& filename);

    ~zz_data_log();

    /**
     * filename of the ULog file
     * @return  eg: "test.ulg" --> "test.1.ulg" or "test.2.ulg" --> "test.3.ulg"
     * */
    std::string generateNewFilename(const std::string& filename);

    /**
     * path of the ULog file
     * @return eg: "/tmp/test.ulg" --> "/tmp/test.1.ulg" or "/tmp/test.2.ulg" --> "/tmp/test.3.ulg"
     * */
    std::string generateNewPathOrFilename(const std::string& pathOrFilename);

    /**
     * Constructor to write to a file.
     * @param filename ULog file to write to (will be overwritten if it exists)
     * Write a key-value info to the header. Typically used for versioning information.
     * @tparam T one of std::string, int32_t, float
     * @param key (unique) name, e.g. sys_name
     * @param key_value value
     * @param all_structs all structs
     */
    template <typename T, typename DataVariant>
    bool Init(const std::string& key, const std::string& key_value, const std::vector<DataVariant>& all_structs) {
        if (key.empty() || key_value.empty()) {
            throw UsageException("Filename, key and key_value must not be empty.");
            return false;
        }

        init_params_.key = key;
        init_params_.key_value = key_value;
        writeInfo(key, key_value);
        // Write all structs to message_format
        for (const auto& struct_variant : all_structs) {
            std::visit(
                [&](const auto& struct_ptr) {
                    if (!struct_ptr.messageName().empty() && !struct_ptr.fields().empty()) {
                        printf("%s %d %s %ld\n", __func__, __LINE__, struct_ptr.messageName().c_str(),
                               struct_ptr.fields().size());

                        StructInfo struct_info;
                        struct_info.messageNname = struct_ptr.messageName();
                        struct_info.fields = struct_ptr.fields();
                        init_params_.all_structs.push_back(struct_info);

                        writeMessageFormat(struct_ptr.messageName(), struct_ptr.fields());
                    } else {
                        throw UsageException("All structs must have a message name and fields.");
                    }
                },
                struct_variant);
        }
        // Check header complete
        headerComplete();
        // Write all structs to add_logged_message
        for (const auto& struct_variant : all_structs) {
            std::visit(
                [&](const auto& struct_ptr) {
                    if (!struct_ptr.messageName().empty() && !struct_ptr.fields().empty()) {
                        uint16_t id = writeAddLoggedMessage(struct_ptr.messageName());
                        id_map_[struct_ptr.messageName()] = id;
                    } else {
                        throw UsageException("All structs must have a message name and fields.");
                    }
                },
                struct_variant);
        }
        printf("Logger Init called.\n");
        return true;
    }
    bool Init(const InitParams& init_params) {
        if (init_params.key.empty() || init_params.key_value.empty()) {
            throw UsageException("Filename, key and key_value must not be empty.");
            return false;
        }
        writeInfo(init_params.key, init_params.key_value);
        // Write all structs to message_format
        for (const auto& struct_variant : init_params.all_structs) {
            writeMessageFormat(struct_variant.messageNname, struct_variant.fields);
        }
        // Check header complete
        headerComplete();
        // Write all structs to add_logged_message
        for (const auto& struct_variant : init_params.all_structs) {
            uint16_t id = writeAddLoggedMessage(struct_variant.messageNname);
            id_map_[struct_variant.messageNname] = id;
        }
        printf("Logger Init called.\n");
        return true;
    }
    /**
     * Create a zz_data_log instance
     * @param filename ULog file to write to (will be overwritten if it exists)
     * @return zz_data_log instance
     */
    static void CreateInstance(const std::string& filename, bool ZzDataLogOn = true);

    /**
     * Get the zz_data_log instance
     */
    static std::shared_ptr<zz_data_log> GetInstance();

    template <typename T>
    void Write(const T data) {
        static int count = 0;
        std::lock_guard<std::mutex> lock(mutex_);
        uint16_t id = 0;
        std::unordered_map<std::string, uint16_t>::iterator it = id_map_.find(data.messageName());
        if (it != id_map_.end()) {
            id = it->second;
        } else {
            throw UsageException("id not found");
        }
        // printf("%s %d %s %d\n", __func__, __LINE__, data.messageName().c_str(), id);
        if (ZzDataLogOn_) {
            writeData(id, data);
            if (count++ == 10) {
                count = 0;
                fsync();
            }
        }
        // printf("Logger Write called.\n");
    }

    /**
     * Write a key-value info to the header. Typically used for versioning information.
     * @tparam T one of std::string, int32_t, float
     * @param key (unique) name, e.g. sys_name
     * @param value
     */
    template <typename T>
    void writeInfo(const std::string& key, const T& value) {
        if (_header_complete) {
            throw UsageException("Header already complete");
        }
        _writer->messageInfo(ulog_cpp::MessageInfo(key, value));
    }

    /**
     * Write a parameter name-value pair to the header
     * @tparam T one of int32_t, float
     * @param key (unique) name, e.g. PARAM_A
     * @param value
     */
    template <typename T>
    void writeParameter(const std::string& key, const T& value) {
        if (_header_complete) {
            throw UsageException("Header already complete");
        }
        _writer->parameter(ulog_cpp::Parameter(key, value));
    }

    /**
     * Write a message format definition to the header.
     *
     * Supported field types:
     * "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t",
     * "float", "double", "bool", "char"
     *
     * The first field must be: {"uint64_t", "timestamp"}.
     *
     * Note that ULog also supports nested format definitions, which is not supported here.
     *
     * When aligning the fields according to a multiple of their size, there must be no padding
     * between fields. The simplest way to achieve this is to order fields by decreasing size of
     * their type. If incorrect, a UsageException() is thrown.
     *
     * @param name format name, must match the regex: "[a-zA-Z0-9_\\-/]+"
     * @param fields message fields, names must match the regex: "[a-z0-9_]+"
     */
    void writeMessageFormat(const std::string& name, const std::vector<Field>& fields);

    /**
     * Call this to complete the header (after calling the above methods).
     */
    void headerComplete();

    /**
     * Write a parameter change (@see writeParameter())
     */
    template <typename T>
    void writeParameterChange(const std::string& key, const T& value) {
        if (!_header_complete) {
            throw UsageException("Header not yet complete");
        }
        _writer->parameter(ulog_cpp::Parameter(key, value));
    }

    /**
     * Create a time-series instance based on a message format definition.
     * @param message_format_name Format name from writeMessageFormat()
     * @param multi_id Instance id, if there's multiple
     * @return message id, used for writeData() later on
     */
    uint16_t writeAddLoggedMessage(const std::string& message_format_name, uint8_t multi_id = 0);

    /**
     * Write a text message
     */
    void writeTextMessage(Logging::Level level, const std::string& message, uint64_t timestamp);

    /**
     * Write some data. The timestamp must be monotonically increasing for a given time-series (i.e.
     * same id).
     * @param id ID from writeAddLoggedMessage()
     * @param data data according to the message format definition
     */
    template <typename T>
    void writeData(uint16_t id, const T& data) {
        writeDataImpl(id, reinterpret_cast<const uint8_t*>(&data), sizeof(data));
    }

    /**
     * Flush the buffer and call fsync() on the file (only if the file-based constructor is used).
     */
    void fsync();

   private:
    static const std::string kFormatNameRegexStr;
    static const std::regex kFormatNameRegex;
    static const std::string kFieldNameRegexStr;
    static const std::regex kFieldNameRegex;

    struct Format {
        unsigned message_size;
    };
    struct Subscription {
        unsigned message_size;
    };

    void writeDataImpl(uint16_t id, const uint8_t* data, unsigned length);

    std::unique_ptr<Writer> _writer;
    std::FILE* _file{nullptr};

    bool _header_complete{false};
    std::unordered_map<std::string, Format> _formats;
    std::vector<Subscription> _subscriptions;

    static std::shared_ptr<zz_data_log> instance_;
    std::unordered_map<std::string, uint16_t> id_map_;
    std::mutex mutex_;
    bool ZzDataLogOn_;

    // 文件大小限制，以字节为单位
    static constexpr uint64_t kMaxFileSize = 10 * 1024 * 1024;  // 10MB
    uint64_t _currentFileSize = 0;

    // 缓存初始化参数
    InitParams init_params_;
};

}  // namespace ulog_cpp
