#ifndef FORMATTER_H
#define FORMATTER_H

#include <sstream>
#include <vector>
#include <functional>
#include <iomanip>
#include <regex>
#include <chrono>

#include "lib/date/date.h"

#include "common.h"
#include "sink.h"

namespace streamlogger {

class pattern {

    using outputter = std::function<void(sink&, const message_info&)>;

    std::vector<outputter> pre;
    std::vector<outputter> post;

    static outputter putLiteral(const std::string& s) {
        return [s](sink& ostr, const message_info&) {
            ostr << s;
        };
    }

    static outputter putPercent() {
        return [](sink& ostr, const message_info&) {
            ostr << '%';
        };
    }

    static constexpr int abs(int v) { return v < 0 ? -v : v; } 

    static void handleMinWidth(sink& ostr, int min_width) {
        if(min_width > 0) ostr << std::left;
        if(min_width != 0) ostr << std::setw(abs(min_width));
    }
    static void writeString(sink& ostr, ::essentials::string_view sv, int min_width, unsigned max_width) {
        handleMinWidth(ostr, min_width);
        if(max_width != 0) {
            ostr << sv.substr(0, max_width);
        } else ostr << sv;
    }

    static outputter putCategory(int min_width, unsigned max_width, const std::string&) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            writeString(ostr, mi.category, min_width, max_width);
        };
    }

    static outputter putCaller(int min_width, unsigned max_width, const std::string&) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            writeString(ostr, mi.caller, min_width, max_width);
        };
    }

    static outputter putDate(int min_width, unsigned max_width, const std::string& postfix) {
        return [min_width, max_width, postfix](sink& ostr, const message_info& mi) {
            auto pfix = postfix;
            if(pfix.empty()) pfix = "%F %T";
            handleMinWidth(ostr, min_width);
            ostr << date::format(pfix, std::chrono::system_clock::now() + util::local_tz_offset());
        };
    }

    static outputter putFilename(int min_width, unsigned max_width, const std::string&) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            writeString(ostr, mi.caller_location.file, min_width, max_width);
        };
    }

    static outputter putLinenumber(int min_width, unsigned max_width, const std::string&) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            handleMinWidth(ostr, min_width);
            ostr << mi.caller_location.line ;
        };
    }

    static outputter putLinefeed(int min_width, unsigned max_width, const std::string& postfix) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            handleMinWidth(ostr, min_width);
            ostr << '\n';
        };
    }

    static outputter putLocation(int min_width, unsigned max_width, const std::string& postfix) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            std::stringstream locus;
            locus << mi.caller_location.file
                  << ":" << mi.caller_location.line
                  << ":" << mi.caller_location.col << std::flush;

            handleMinWidth(ostr, min_width);
            ostr << locus.rdbuf();
        };
    }

    static outputter putPriority(int min_width, unsigned max_width, const std::string& postfix) {
        return [min_width, max_width](sink& ostr, const message_info& mi) {
            const char* prio;
            switch(mi.level) {
                case level::ALL:
                    prio = "ALL";
                    break;
                case level::TRACE:
                    prio = "TRACE";
                    break;
                case level::DEBUG:
                    prio = "DEBUG";
                    break;
                case level::INFO:
                    prio = "INFO";
                    break;
                case level::WARN:
                    prio = "WARN";
                    break;
                case level::ERROR:
                    prio = "ERROR";
                    break;
                case level::FATAL:
                    prio = "FATAL";
                    break;
            }

            writeString(ostr, prio, min_width, max_width);
        };
    }

    pattern() = default;

public:
    pattern(const pattern&) = default;
    pattern(pattern&&) = default;

    static pattern parse(const std::string& rep) {
        std::istringstream istr(rep);

        bool messageDone = false;
        pattern pat;

        auto toInsert = [&]()-> std::vector<outputter>& { return messageDone? pat.post : pat.pre; };

        std::string literal;
        std::string postfix;
        int min_width = 0;
        unsigned max_width = 0;

        while(istr) {
            std::getline(istr, literal, '%');
            toInsert().push_back(putLiteral(literal));
            if(istr.eof()) break;

            char ch;
            istr.get(ch);

            if(ch == '%') {
                toInsert().push_back(putPercent());
                continue;
            }

            // preamble '-'?[0-9]*'.'?[0-9]*
            min_width = 0;
            if(ch == '-' || ('0' < ch && ch < '9')) {
                istr.putback(ch);
                istr >> min_width;
                istr.get(ch);
            }

            max_width = 0;
            if(ch == '.') {
                istr >> max_width;
                istr.get(ch);
            }

            char code = ch;

            postfix = "";
            istr.get(ch);
            if(ch == '{') {
                getline(istr, postfix, '}');
            } else {
                istr.putback(ch);
            }

            switch(code) {
                case 'c': {
                    toInsert().push_back(putCategory(min_width, max_width, postfix));
                    break;
                }
                case 'C': {
                    toInsert().push_back(putCaller(min_width, max_width, postfix));
                    break;
                }
                case 'd': {
                    toInsert().push_back(putDate(min_width, max_width, postfix));
                    break;
                }
                case 'p': {
                    toInsert().push_back(putPriority(min_width, max_width, postfix));
                    break;
                }
                case 'F': {
                    toInsert().push_back(putFilename(min_width, max_width, postfix));
                    break;
                }
                case 'l': {
                    toInsert().push_back(putLocation(min_width, max_width, postfix));
                    break;
                }
                case 'L': {
                    toInsert().push_back(putLinenumber(min_width, max_width, postfix));
                    break;
                }
                case 'm': {
                    messageDone = true;
                    break;
                }
                case 'M': {
                    toInsert().push_back(putCaller(min_width, max_width, postfix));
                }
                case 'n': {
                    toInsert().push_back(putLinefeed(min_width, max_width, postfix));
                }
                default: throw std::runtime_error("Incorrect pattern specified: " + rep);
            }
        }

        return std::move(pat);
    }

    void print_prefix(sink& ost, const message_info* mi) {
        for(auto&& f : pre) {
            f(ost, *mi);
        }
    }

    void print_suffix(sink& ost, const message_info* mi) {
        for(auto&& f : post) {
            f(ost, *mi);
        }
    }

};

class formatter {
    std::shared_ptr<sink> sink_;
    pattern pattern;
    level threshold = level::TRACE;
    bool skip = false;
public:
    formatter(std::shared_ptr<sink> sink, const std::string& pstring, level threshold = level::ALL)
        : sink_(sink), pattern(pattern::parse(pstring)), threshold(threshold) {}

    formatter& operator<<(const message_start& ms) {
        skip = ms.info->level < threshold;
        if(not skip) {
            (*sink_) << ms;
            pattern.print_prefix(*sink_, ms.info);
        }
        return *this;
    }

    formatter& operator<<(const message_end& ms) {
        if(not skip) {
            pattern.print_suffix(*sink_, ms.info);
            (*sink_) << ms;
        }
        return *this;
    }

    template<
        class T,
        class = std::enable_if_t<
               not std::is_same<std::decay_t<T>, message_start>::value
            && not std::is_same<std::decay_t<T>, message_end>::value
        >
    >
    formatter& operator<<(T&& value) {
        if(not skip) (*sink_) << std::forward<T>(value);
        return *this;
    }
};

} /* namespace streamlogger */

#endif // FORMATTER_H
