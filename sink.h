#ifndef SINK_H
#define SINK_H

#include <mutex>
#include <fstream>
#include <bits/unordered_map.h>
#include "common.h"

namespace streamlogger {

class sink {
protected:
    std::ostream* stream;
    bool owns_stream;
    std::mutex sink_mutex;

    virtual void handle_start(const message_info& mi) = 0;
    virtual void handle_end(const message_info& mi) = 0;

    sink(std::ostream *stream, bool owns_stream): stream(stream), owns_stream(owns_stream), sink_mutex{} {}
    virtual ~sink() {
        if(owns_stream) delete stream;
    }

    template<
        class T,
        class = std::enable_if_t<
            not std::is_same<std::decay_t<T>, message_start>::value
         && not std::is_same<std::decay_t<T>, message_end>::value
        >
    >
    void output(T&& value) {
        (*stream) << std::forward<T>(value);
    }

    void output(const message_start& m) {
        handle_start(*m.info);
        sink_mutex.lock();
    }

    void output(const message_end& m) {
        sink_mutex.unlock();
        handle_end(*m.info);
    }

public:
    sink() = delete;
    sink(const sink&) = delete;

    template<class T>
    sink& operator<<(T&& value) {
        output(std::forward<T>(value));
        return *this;
    }


};

class cout_sink: public sink {
    void handle_start(const message_info&) override {}
    void handle_end(const message_info&) override {
        (*stream) << '\n';
    }

public:
    cout_sink(): sink(&std::cout, false) {}
    virtual ~cout_sink() = default;

    static std::shared_ptr<sink> instance() {
        static auto result = std::make_shared<cout_sink>();
        return result;
    }
};

class cerr_sink: public sink {
    void handle_start(const message_info&) override {}
    void handle_end(const message_info&) override {
        (*stream) << '\n';
    }

public:
    cerr_sink(): sink(&std::cerr, false) {}
    virtual ~cerr_sink() = default;

    static std::shared_ptr<sink> instance() {
        static auto result = std::make_shared<cerr_sink>();
        return result;
    }
};

class file_sink: public sink {
public:
    enum class mode{ APPEND, TRUNCATE };

private:
    void handle_start(const message_info&) override {}
    void handle_end(const message_info& mi) override {
        (*stream) << '\n';
    }

    template<class Name>
    static std::ofstream* open_file(Name&& name, mode m) {
        std::ios::openmode mode = std::ios::out;
        if(m == mode::APPEND) mode |= std::ios::app;
        if(m == mode::TRUNCATE) mode |= std::ios::trunc;
        return new std::ofstream(name, mode);
    }
public:
    file_sink(const std::string& filename, mode m = mode::APPEND):
        sink(open_file(filename, m), true) {}
    file_sink(const char* filename, mode m = mode::APPEND):
        sink(open_file(filename, m), true) {}

    static std::shared_ptr<sink> instance(const std::string& filename) {
        static std::unordered_map<std::string, std::shared_ptr<sink>> registry;
        auto it = registry.find(filename);
        if(it == registry.end()) {
            return registry[filename] = std::make_shared<file_sink>(filename);
        } else return it->second;
    }
};

} /* namespace streamlogger */

#endif // SINK_H
