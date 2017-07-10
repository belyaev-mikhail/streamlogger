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

        (*multiplexer_) << message_start{&mi};
    }

    ~logger() {
        (*multiplexer_) << message_end{&mi};
    }

    template <class T>
    logger& operator<<(T&& value) {
        (*multiplexer_) << std::forward<T>(value);
        return *this;
    }

};

} /* namespace streamlogger */

#endif // LOGGER_H
