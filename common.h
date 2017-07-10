#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>

namespace streamlogger {

enum class level {
    ALL,
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

struct location {
    std::string file = "unknown file";
    size_t line = ~size_t(0);
    size_t col = ~size_t(0);
};

struct message_info {
    std::string category;
    level level;
    std::chrono::system_clock::time_point time_point = std::chrono::system_clock::now();
    std::thread::id thread_id = std::this_thread::get_id();

    // caller information (if available)
    std::string caller = "unknown function";
    location caller_location;
};

struct message_start {
    message_info* info;
};

struct message_end {
    message_info* info;
};

} /* namespace streamlogger */

#endif // COMMON_H
