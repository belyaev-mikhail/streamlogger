#ifndef CATEGORY_H
#define CATEGORY_H

#include "common.h"
#include "multiplexer.h"
#include "logger.h"

#include <string>

namespace streamlogger {

class category {
    std::string name_;
    std::shared_ptr<multiplexer> multiplexer_;
public:
    explicit category(const std::string& name): name_(name), multiplexer_(new multiplexer()) {}

    void add_sink(std::shared_ptr<sink> out, const std::string& pattern, level threshold = level::ALL) {
        auto form = std::make_shared<formatter>(out, pattern, threshold);
        multiplexer_->formatters.push_back(form);
    }

    streamlogger::logger logger(level level_) {
        return streamlogger::logger{ name_, level_, multiplexer_, nullptr, nullptr };
    }

    streamlogger::logger trace() { return logger(level::TRACE); }
    streamlogger::logger debug() { return logger(level::DEBUG); }
    streamlogger::logger info()  { return logger(level::INFO);  }
    streamlogger::logger warn()  { return logger(level::WARN);  }
    streamlogger::logger error() { return logger(level::ERROR); }
    streamlogger::logger fatal() { return logger(level::FATAL); }
};

} /* namespace streamlogger */

#endif // CATEGORY_H
