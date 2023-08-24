/****************************************************************************
 * Copyright (c) 2023 PX4 Development Team.
 * SPDX-License-Identifier: BSD-3-Clause
 ****************************************************************************/

#include "zz_data_log.hpp"

#include <unistd.h>

std::shared_ptr<zz_data_log> zz_data_log::instance_ = nullptr;

namespace ulog_cpp {

const std::string zz_data_log::kFormatNameRegexStr = "[a-zA-Z0-9_\\-/]+";
const std::regex zz_data_log::kFormatNameRegex = std::regex(std::string(kFormatNameRegexStr));
const std::string zz_data_log::kFieldNameRegexStr = "[a-zA-Z0-9_]+";
const std::regex zz_data_log::kFieldNameRegex = std::regex(std::string(kFieldNameRegexStr));

bool isValidFilename(const std::string& filename) {
    // 查找文件名中的最后一个'.'
    size_t lastDotPos = filename.rfind('.');

    // 如果没找到'.'或者后缀不是.ulg,返回false
    if (lastDotPos == std::string::npos || filename.substr(lastDotPos) != ".ulg") {
        return false;
    }

    // 找到'.ulg'后缀,返回true
    return true;
}

void zz_data_log::CreateInstance(const std::string& filename, bool ZzDataLogOn) {
    if (filename.empty()) {
        throw UsageException("Filename must not be empty.");
    }
    if (!zz_data_log::instance_) {
        zz_data_log::instance_ = std::make_shared<zz_data_log>(filename);
    }
    instance_->ZzDataLogOn_ = ZzDataLogOn;
    printf("Logger CreateInstance called.\n");
}

std::shared_ptr<zz_data_log> zz_data_log::GetInstance() {
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> lock(instanceMutex);

    if (!zz_data_log::instance_) {
        throw UsageException("Logger not initialized, please CreateInstance() first.");
    }
    // printf("Logger GetInstance called.\n");
    return zz_data_log::instance_;
}

zz_data_log::zz_data_log(DataWriteCB data_write_cb, uint64_t timestamp_us)
    : _writer(std::make_unique<Writer>(std::move(data_write_cb))) {
    _writer->fileHeader(FileHeader(timestamp_us));
}

zz_data_log::zz_data_log(const std::string& filename, uint64_t timestamp_us) {
    _file = std::fopen(filename.c_str(), "wb");
    if (!_file) {
        throw ParsingException("Failed to open file");
    }

    _writer =
        std::make_unique<Writer>([this](const uint8_t* data, int length) { std::fwrite(data, 1, length, _file); });
    _writer->fileHeader(FileHeader(timestamp_us));
}

zz_data_log::zz_data_log(const std::string& filename) {
    if (!isValidFilename(filename)) {
        throw UsageException(
            "Invalid filename, please input a valid filename, test.ulg or test.1.ulg or /tmp/test.ulg or "
            "/tmp/test.1.ulg");
    }
    init_params_.file_name = filename;
    _file = std::fopen(filename.c_str(), "wb");
    if (!_file) {
        throw ParsingException("Failed to open file");
    }

    _writer =
        std::make_unique<Writer>([this](const uint8_t* data, int length) { std::fwrite(data, 1, length, _file); });
    _writer->fileHeader(FileHeader(currentTimeUs()));
}

zz_data_log::~zz_data_log() {
    _writer.reset();
    if (_file) {
        std::fclose(_file);
    }
}

std::string zz_data_log::generateNewFilename(const std::string& filename) {
    int version = 0;
    std::cout << "generateNewFilename: " << filename << std::endl;
    if (filename.empty()) {
        throw UsageException("Filename must not be empty.");
    }
    // 寻找文件名中的最后一个点的位置
    size_t lastDotPos = filename.rfind('.');
    if (lastDotPos == std::string::npos) {
        // 如果找不到点，直接返回原始路径
        return filename;
    }

    // 找到版本号开始的点
    size_t versionStartPos = filename.rfind('.', lastDotPos - 1);
    if (versionStartPos == lastDotPos) {
        version = 1;
    } else {
        // 提取版本号
        std::string versionStr = filename.substr(versionStartPos + 1, lastDotPos - versionStartPos - 1);
        std::cout << "versionStr: " << versionStr << std::endl;
        version = std::stoi(versionStr);
        ++version;
    }

    std::string newFilename = filename.substr(0, versionStartPos) + "." + std::to_string(version) + ".ulg";
    return newFilename;
}

std::string zz_data_log::generateNewPathOrFilename(const std::string& pathOrFilename) {
    int version = 0;
    std::cout << "generateNewPathOrFilename: " << pathOrFilename << std::endl;
    if (pathOrFilename.empty()) {
        throw UsageException("Path or filename must not be empty.");
    }
    // 寻找最后一个斜杠的位置
    size_t lastSlashPos = pathOrFilename.rfind('/');
    if (lastSlashPos == std::string::npos) {
        // 如果找不到斜杠，直接进行文件名处理
        return generateNewFilename(pathOrFilename);
    }

    // 从路径中提取文件名部分
    std::string filename = pathOrFilename.substr(lastSlashPos + 1);

    // 寻找文件名中的最后一个点的位置
    size_t lastDotPos = filename.rfind('.');
    if (lastDotPos == std::string::npos) {
        // 如果找不到点，直接返回原始路径
        return pathOrFilename;
    }

    // 找到版本号开始的点
    size_t versionStartPos = filename.rfind('.', lastDotPos - 1);
    if (versionStartPos == lastDotPos) {
        version = 1;
    } else {
        // 提取版本号
        std::string versionStr = filename.substr(versionStartPos + 1, lastDotPos - versionStartPos - 1);
        std::cout << "versionStr: " << versionStr << std::endl;
        version = std::stoi(versionStr);
        // 递增版本号
        ++version;
    }

    // 构建新的文件名
    std::string newFilename = filename.substr(0, versionStartPos) + "." + std::to_string(version) + ".ulg";

    // 构建新的路径
    std::string newPath = pathOrFilename.substr(0, lastSlashPos + 1) + newFilename;
    return newPath;
}

void zz_data_log::writeMessageFormat(const std::string& name, const std::vector<Field>& fields) {
    if (_header_complete) {
        throw UsageException("Header already complete");
    }
    // Ensure the first field is the 64 bit timestamp. This is a bit stricter than what ULog requires
    if (fields.empty() || fields[0].name != "timestamp" || fields[0].type != "uint64_t" ||
        fields[0].array_length != -1) {
        throw UsageException("First message field must be 'uint64_t timestamp'");
    }
    if (_formats.find(name) != _formats.end()) {
        throw UsageException("Duplicate format: " + name);
    }

    // Validate naming pattern
    if (!std::regex_match(name, kFormatNameRegex)) {
        throw UsageException("Invalid name: " + name + ", valid regex: " + kFormatNameRegexStr);
    }
    for (const auto& field : fields) {
        if (!std::regex_match(field.name, kFieldNameRegex)) {
            throw UsageException("Invalid field name: " + field.name + ", valid regex: " + kFieldNameRegexStr);
        }
    }

    // Check field types and verify padding
    unsigned message_size = 0;
    for (const auto& field : fields) {
        const auto& basic_type_iter = Field::kBasicTypes.find(field.type);
        if (basic_type_iter == Field::kBasicTypes.end()) {
            throw UsageException("Invalid field type (nested formats are not supported): " + field.type);
        }
        const int array_size = field.array_length <= 0 ? 1 : field.array_length;
        if (message_size % basic_type_iter->second != 0) {
            throw UsageException(
                "struct requires padding, reorder fields by decreasing type size. Padding before "
                "field: " +
                field.name);
        }
        message_size += array_size * basic_type_iter->second;
    }
    _formats[name] = Format{message_size};
    _writer->messageFormat(MessageFormat(name, fields));
}

void zz_data_log::headerComplete() {
    if (_header_complete) {
        throw UsageException("Header already complete");
    }
    _writer->headerComplete();
    _header_complete = true;
}

void zz_data_log::writeTextMessage(Logging::Level level, const std::string& message, uint64_t timestamp) {
    if (!_header_complete) {
        throw UsageException("Header not yet complete");
    }
    _writer->logging({level, message, timestamp});
}

void zz_data_log::fsync() {
    if (_file) {
        fflush(_file);
        ::fsync(fileno(_file));
    }
}
uint16_t zz_data_log::writeAddLoggedMessage(const std::string& message_format_name, uint8_t multi_id) {
    if (!_header_complete) {
        throw UsageException("Header not yet complete");
    }
    const uint16_t msg_id = _subscriptions.size();
    auto format_iter = _formats.find(message_format_name);
    if (format_iter == _formats.end()) {
        throw UsageException("Format not found: " + message_format_name);
    }
    _subscriptions.push_back({format_iter->second.message_size});
    _writer->addLoggedMessage(AddLoggedMessage(multi_id, msg_id, message_format_name));
    return msg_id;
}

void zz_data_log::writeDataImpl(uint16_t id, const uint8_t* data, unsigned length) {
    if (!_header_complete) {
        throw UsageException("Header not yet complete");
    }
    if (id >= _subscriptions.size()) {
        throw UsageException("Invalid ID");
    }
    const unsigned expected_size = _subscriptions[id].message_size;
    // Sanity check data size. sizeof(data) can be bigger because of struct padding at the end
    if (length < expected_size) {
        throw UsageException("sizeof(data) is too small");
    }
    std::vector<uint8_t> data_vec;
    data_vec.resize(expected_size);
    memcpy(data_vec.data(), data, expected_size);

    _currentFileSize += length;

    if (_currentFileSize >= kMaxFileSize) {
        // 超过文件大小限制，关闭当前文件，生成新文件
        _writer.reset();
        std::fclose(_file);

        _header_complete = false;
        _subscriptions.clear();
        _formats.clear();
        id_map_.clear();

        // 生成新文件名
        std::string newFilename = generateNewPathOrFilename(init_params_.file_name);
        init_params_.file_name = newFilename;
        _file = std::fopen(newFilename.c_str(), "wb");
        if (!_file) {
            throw ParsingException("Failed to open file");
        }

        _currentFileSize = 0;
        // 创建新的 _writer
        _writer =
            std::make_unique<Writer>([this](const uint8_t* data, int length) { std::fwrite(data, 1, length, _file); });
        // 重新写入文件头
        _writer->fileHeader(FileHeader(currentTimeUs()));
        // 重新写入格式信息等

        if (!Init(init_params_)) {
            throw UsageException("Init failed." + init_params_.file_name + "need restart.");
        }
    }

    _writer->data(Data(id, std::move(data_vec)));
}

}  // namespace ulog_cpp
