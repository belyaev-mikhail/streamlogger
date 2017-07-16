#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <ctime>
#include <cmath>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>

#include "lib/string_view/string_view.hpp"

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
    std::chrono::system_clock::time_point time_point;
    std::thread::id thread_id;

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

namespace util {

template<class Char, size_t N>
essentials::basic_string_view<Char> view(const Char (&arr)[N]) {
    return essentials::basic_string_view<Char>(arr, N-1);
};

inline essentials::string_view trim_start(essentials::string_view sv) {
    auto start = sv.find_first_not_of(" \t\n\r");
    if(start == essentials::string_view::npos) return "";
    if(start == 0) return sv;
    return sv.substr(start);
}

inline essentials::string_view trim_end(essentials::string_view sv) {
    auto end = sv.find_last_not_of(" \t\n\r");
    if(end == essentials::string_view::npos) return "";
    else return sv.substr(0, end + 1);
}

inline essentials::string_view trim(essentials::string_view sv) {
    return trim_end(trim_start(sv));
}

class tokenizer {
    essentials::string_view tokens;
    essentials::string_view sv;

public:
    tokenizer(essentials::string_view tokens, essentials::string_view sv): tokens(tokens), sv(sv) {}

    bool has_next() {
        return not sv.empty();
    }

    essentials::string_view next() {
        auto where = sv.find_first_of(tokens);
        auto res = sv.substr(0, where);
        if(where == essentials::string_view::npos) sv = "";
        else sv.remove_prefix(where + 1);
        return res;
    }
};

inline std::chrono::seconds local_tz_offset() {
    using namespace std;
    using namespace std::chrono;

    static seconds res = []() -> seconds {
        time_t t = time(nullptr);
        struct tm lt_ = {0};
        struct tm gt_ = {0};

        lt_ = *localtime(&t);
        gt_ = *gmtime(&t);

        return seconds(lround(difftime(mktime(&lt_), mktime(&gt_))));
    }();
    return res;
}

} /* namespace util */

template<class Char, class Traits = std::char_traits<Char>>
std::basic_ostream<Char, Traits> operator<<(std::basic_ostream<Char, Traits>& os, level lvl) {
    switch(lvl) {
        case level::ALL: return os << "ALL";
        case level::TRACE: return os << "TRACE";
        case level::DEBUG: return os << "DEBUG";
        case level::INFO: return os << "INFO";
        case level::WARN: return os << "WARN";
        case level::ERROR: return os << "ERROR";
        case level::FATAL: return os << "FATAL";
    }
};

template<class Char, class Traits = std::char_traits<Char>>
level parse_level(const Char* ch) {
    if(util::view("TRACE") == ch) return level::TRACE;
    if(util::view("DEBUG") == ch) return level::DEBUG;
    if(util::view("INFO") == ch) return level::INFO;
    if(util::view("WARN") == ch) return level::WARN;
    if(util::view("ERROR") == ch) return level::ERROR;
    if(util::view("FATAL") == ch) return level::FATAL;
    return level::ALL;
}

} /* namespace streamlogger */

#endif // COMMON_H
