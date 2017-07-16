#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <string>
#include <unordered_map>
#include "lib/inih/INIReader.h"

#include "category.h"

namespace streamlogger {

class registry {
    static std::shared_ptr<::streamlogger::category> root() {
        static std::shared_ptr<::streamlogger::category> root_ = std::make_shared<::streamlogger::category>("");
        return root_;
    }

    static std::unordered_map<std::string, std::shared_ptr<::streamlogger::category>>& data() {
        static std::unordered_map<std::string, std::shared_ptr<::streamlogger::category>> data_;
        return data_;
    };

    struct appender {
        std::string type;
        std::string filename;
        std::string pattern;
        std::string threshold;
    };

    struct category {
        bool additive = true;
        std::vector<std::string> appenders;
    };

    struct parse_state {
        std::unordered_map<std::string, appender> formatters;
        std::unordered_map<std::string, category> categories;
    };

    static int ini_handler(void* user, const char* section, const char* name, const char* value) {
        auto&& parse_state = *static_cast<struct parse_state*>(user);

        util::tokenizer name_split(".", name);

        auto keyword = util::trim(name_split.next());
        if(keyword == "appender") {
            auto appender_name = util::trim(name_split.next());
            if(not name_split.has_next()) {
                parse_state.formatters[appender_name].type = util::trim(value);
                return 0;
            }
            auto field = util::trim(name_split.next());
            if(field == "fileName") {
                parse_state.formatters[appender_name].filename = util::trim(value);
                return 0;
            }

            if(field == "layout" && not name_split.has_next()) return 0; //ignore
            if(field == "layout") {
                auto patternField = util::trim(name_split.next());
                if(patternField != "ConversionPattern") return -1;
                parse_state.formatters[appender_name].pattern = util::trim(value);
                return 0;
            }

            if(field == "threshold") {
                parse_state.formatters[appender_name].threshold = util::trim(value);
                return 0;
            }
            return -1; // nothing else supported atm
        }

        if(keyword == "category" || keyword == "rootCategory") {
            auto categoryName = util::trim(name_split.next());
            util::tokenizer value_split(",", value);
            auto level = util::trim(value_split.next());
            while(value_split.has_next()) {
                parse_state.categories[categoryName].appenders.push_back(util::trim(value_split.next()));
            }
            return 0; // nothing else supported atm
        }

        if(keyword == "additivity") {
            auto categoryName = util::trim(name_split.next());
            parse_state.categories[categoryName].additive = not (util::trim(value) == "false");
            return 0; // nothing else supported atm
        }

        return -1;
    }

public:
    static void configure(const std::string& logini) {
        parse_state ps;
        ini_parse(logini.c_str(), ini_handler, &ps);

        std::unordered_map<std::string, std::shared_ptr<sink>> sinks;
        for(auto&& ap : ps.formatters) {
            if(ap.second.type == "FileAppender") {
                sinks[ap.first] = file_sink::instance(ap.second.filename);
            }
            if(ap.second.type == "ConsoleAppender") {
                sinks[ap.first] = cout_sink::instance();
            }
        }

        for(auto&& cat : ps.categories) {
            data()[cat.first] = std::make_shared<::streamlogger::category>(cat.first);
            for(auto&& ap : cat.second.appenders) {
                data()[cat.first]->add_sink(
                    sinks[ap],
                    ps.formatters[ap].pattern,
                    parse_level(ps.formatters[ap].threshold.c_str())
                );
            }
        }

    }

    static logger getLogger(const std::string& category, level lvl) {
        auto it = data().find(category);
        if(it == data().end()) return getLogger("", lvl);
        else return it->second->logger(lvl);
    }

    static logger all(const std::string& category) { return getLogger(category, level::ALL); }
    static logger trace(const std::string& category) { return getLogger(category, level::TRACE); }
    static logger debug(const std::string& category) { return getLogger(category, level::DEBUG); }
    static logger info(const std::string& category) { return getLogger(category, level::INFO); }
    static logger warn(const std::string& category) { return getLogger(category, level::WARN); }
    static logger error(const std::string& category) { return getLogger(category, level::ERROR); }
    static logger fatal(const std::string& category) { return getLogger(category, level::FATAL); }

};

static void configure(const std::string& logini) {
    registry::configure(logini);
}

static logger getLogger(const std::string& category, level lvl) {
    return registry::getLogger(category, lvl);
}

static logger all(const std::string& category) { return registry::all(category); }
static logger trace(const std::string& category) { return registry::trace(category); }
static logger debug(const std::string& category) { return registry::debug(category); }
static logger info(const std::string& category) { return registry::info(category); }
static logger warn(const std::string& category) { return registry::warn(category); }
static logger error(const std::string& category) { return registry::error(category); }
static logger fatal(const std::string& category) { return registry::fatal(category); }

} /* namespace streamlogger */

#endif // CONFIGURATOR_H
