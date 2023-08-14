#pragma once
#include <mutex>

#include "simple_writer.hpp"
using namespace std;
using namespace ulog_cpp;
using namespace std::chrono_literals;

extern bool ZzDataLogOn;

class zz_data_log {
   public:
    // Declare CreateInstance as a friend function
    friend void CreateInstance(const std::string& filename);
    friend std::shared_ptr<zz_data_log> GetInstance();
    /**
     * zz_data_log Constructor for writing data.
     * @param filename ULog file to write to (will be overwritten if it exists)
     */
    explicit zz_data_log(const std::string& filename);

    /**
     * zz_data_log Destructor
     */
    ~zz_data_log();

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
        writer_->writeInfo(key, key_value);
        // Write all structs to message_format
        for (const auto& struct_variant : all_structs) {
            std::visit(
                [&](const auto& struct_ptr) {
                    if (!struct_ptr.messageName().empty() && !struct_ptr.fields().empty()) {
                        printf("%s %d %s %ld\n", __func__, __LINE__, struct_ptr.messageName().c_str(),
                               struct_ptr.fields().size());

                        writer_->writeMessageFormat(struct_ptr.messageName(), struct_ptr.fields());
                    } else {
                        throw UsageException("All structs must have a message name and fields.");
                    }
                },
                struct_variant);
        }
        // Check header complete
        writer_->headerComplete();
        // Write all structs to add_logged_message
        for (const auto& struct_variant : all_structs) {
            std::visit(
                [&](const auto& struct_ptr) {
                    if (!struct_ptr.messageName().empty() && !struct_ptr.fields().empty()) {
                        uint16_t id = writer_->writeAddLoggedMessage(struct_ptr.messageName());
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

    /**
     * Create a zz_data_log instance
     * @param filename ULog file to write to (will be overwritten if it exists)
     * @return zz_data_log instance
     */
    static void CreateInstance(const std::string& filename);

    /**
     * Get the zz_data_log instance
     */
    static std::shared_ptr<zz_data_log> GetInstance();

    /**
     * Write a parameter name-value pair to the header
     * @tparam T one of int32_t, float
     * @param key (unique) name, e.g. PARAM_A
     * @param value
     */
    template <typename T>
    void WriteParameter(const std::string& key, const T& value) {
        writer_->writeParameter(key, value);
        printf("Logger WriteParameter called.\n");
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
    void WriteMessageFormat(const std::string& name, const std::vector<Field>& fields);

    /**
     * Call this to complete the header (after calling the above methods).
     */
    void HeaderComplete();

    /**
     * Create a time-series instance based on a message format definition.
     * @param message_format_name Format name from writeMessageFormat()
     * @param multi_id Instance id, if there's multiple
     * @return message id, used for writeData() later on
     */
    uint16_t WriteAddLoggedMessage(const std::string& message_format_name, uint8_t multi_id = 0);

    /**
     * Write some data. The timestamp must be monotonically increasing for a given time-series (i.e.
     * same id).
     * @param id ID from writeAddLoggedMessage()
     * @param data data according to the message format definition
     */
    // template <typename T>
    // void Write(const T& data);
    template <typename T>
    void Write(const T data) {
        std::lock_guard<std::mutex> lock(mutex_);
        // uint16_t id = id_map_[data.messageName()];
        uint16_t id = 0;
        std::map<std::string, uint16_t>::iterator it = id_map_.find(data.messageName());
        if (it != id_map_.end()) {
            id = it->second;
        } else {
            throw UsageException("id not found");
        }
        printf("%s %d %s %d\n", __func__, __LINE__, data.messageName().c_str(), id);
        if (ZzDataLogOn) {
            writer_->writeData(id, data);
        }
        printf("Logger Write called.\n");
    }
    /**
     * Flush the buffer and call fsync() on the file (only if the file-based constructor is used).
     */
    void Fsync();

   private:
    static std::shared_ptr<zz_data_log> instance_;

    std::unique_ptr<ulog_cpp::SimpleWriter> writer_;
    std::mutex mutex_;
    std::map<std::string, uint16_t> id_map_;
};
