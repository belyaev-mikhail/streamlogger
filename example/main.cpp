#include <streamlogger/category.h>

int main() {
    using namespace streamlogger;
    category cat("base");

    cat.add_sink(cout_sink::instance(), "[%d] %-5p [%-10c] %m %% >>>");
    cat.add_sink(file_sink::instance("errors.log"), "<%-8.8c> %m", level::WARN);

    cat.info() << "Hello my little friend " << 42;
    cat.error() << "Wat the hell is goin' on 'ere?";
}
