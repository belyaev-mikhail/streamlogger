#ifndef LOGGER_H
#define LOGGER_H

#include "common.h"
#include "multiplexer.h"

namespace streamlogger {

class logger {
    const std::string &category_;
    level level_;
    std::shared_ptr<multiplexer> multiplexer_;
    message_info mi;
    bool initialized = false;

public:
    logger(const std::string &category,
           level level,
           std::shared_ptr<multiplexer> multiplexer,
           const char *caller,
           const location *location) :
        category_(category),
        level_(level),
        multiplexer_(multiplexer),
        mi{} {
        mi.category = category_;
        mi.level = level_;
        if (caller) mi.caller = caller;
        if (location) mi.caller_location = *location;
    }

    logger(logger&& that):
        category_(std::move(that.category_)),
        level_(that.level_),
        multiplexer_(std::move(that.multiplexer_)),
        mi(std::move(that.mi)),
        initialized(that.initialized) {

        that.multiplexer_ = nullptr; // just to be sure
    }

    logger& operator=(logger&&) = delete;
    logger& operator=(const logger&) = delete;

    ~logger() {
        if(multiplexer_ && initialized) (*multiplexer_) << message_end{&mi};
    }

    template <class T>
    logger& operator<<(T&& value) {
        if(not initialized) (*multiplexer_) << message_start{&mi};
        initialized = true;

        (*multiplexer_) << std::forward<T>(value);
        return *this;
    }

    void flush() {
        (*multiplexer_) << static_cast<std::add_pointer_t<std::ostream&(std::ostream&)>>(std::flush);
    }

};

} /* namespace streamlogger */

#endif // LOGGER_H
