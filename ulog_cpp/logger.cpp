#include "logger.hpp"

std::shared_ptr<logger> logger::instance_ = nullptr;

/**
 * returns the current time in microseconds
 */
static uint64_t currentTimeUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

void logger::CreateInstance(const std::string& filename) {
    printf("%s %d\n", __func__, __LINE__);
    if (filename.empty()) {
        throw UsageException("Filename must not be empty.");
    }
    if (!logger::instance_) {
        logger::instance_ = std::make_shared<logger>(filename);
    }
    printf("Logger CreateInstance called.\n");
}

std::shared_ptr<logger> logger::GetInstance() {
    static std::mutex instanceMutex;
    std::lock_guard<std::mutex> lock(instanceMutex);
    printf("%s %d\n", __func__, __LINE__);

    if (!logger::instance_) {
        throw UsageException("Logger not initialized, please CreateInstance() first.");
    }
    printf("Logger GetInstance called.\n");
    return logger::instance_;
}

logger::logger(const std::string& filename)
    : writer_(std::make_unique<ulog_cpp::SimpleWriter>(filename, currentTimeUs())) {
    if (filename.empty()) {
        throw UsageException("Filename must not be empty.");
    }
    printf("Logger Constructor called.\n");
}

logger::~logger() {}
#if 0
bool logger::Init(const std::string& key, const std::string& key_value,
                  const std::vector<std::shared_ptr<BaseStruct>>& all_structs) {
    if (key.empty() || key_value.empty()) {
        throw UsageException("Filename, key and key_value must not be empty.");
        return false;
    }
    writer_->writeInfo(key, key_value);
    // Write all structs to message_format
    for (const auto& struct_ptr : all_structs) {
        if (struct_ptr && !struct_ptr->messageName().empty() && !struct_ptr->fields().empty()) {
            printf("%s %d %s %ld\n", __func__, __LINE__, struct_ptr->messageName().c_str(),
                   struct_ptr->fields().size());

            writer_->writeMessageFormat(struct_ptr->messageName(), struct_ptr->fields());
        } else {
            throw UsageException("All structs must have a message name and fields.");
        }
    }
    // Check header complete
    writer_->headerComplete();
    // Write all structs to add_logged_message
    for (const auto& struct_ptr : all_structs) {
        if (struct_ptr && !struct_ptr->messageName().empty() && !struct_ptr->fields().empty()) {
            printf("%s %d %s %ld\n", __func__, __LINE__, struct_ptr->messageName().c_str(),
                   struct_ptr->fields().size());

            uint16_t id = writer_->writeAddLoggedMessage(struct_ptr->messageName());
            id_map_[struct_ptr->messageName()] = id;
        } else {
            throw UsageException("All structs must have a message name and fields.");
        }
    }
    printf("Logger Init called.\n");
    return true;
}
#endif
void logger::WriteMessageFormat(const std::string& name, const std::vector<Field>& fields) {
    writer_->writeMessageFormat(name, fields);
    printf("Logger WriteMessageFormat called.\n");
}

void logger::HeaderComplete() {
    writer_->headerComplete();
    printf("Logger HeaderComplete called.\n");
}

uint16_t logger::WriteAddLoggedMessage(const std::string& message_format_name, uint8_t multi_id) {
    printf("Logger WriteAddLoggedMessage called.\n");
    return writer_->writeAddLoggedMessage(message_format_name, multi_id);
}

void logger::Fsync() {
    std::lock_guard<std::mutex> lock(mutex_);

    writer_->fsync();
    printf("Logger Fsync called.\n");
}