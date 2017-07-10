#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "common.h"
#include "formatter.h"

namespace streamlogger {

class multiplexer {
    std::vector<std::shared_ptr<formatter>> formatters;

    friend class category;
public:
    template<class T>
    multiplexer& operator<<(T&& value) {
        for(auto&& f : formatters) {
            (*f) << value;
        }
        return *this;
    }
};

} /* namespace streamlogger */

#endif // MULTIPLEXER_H
