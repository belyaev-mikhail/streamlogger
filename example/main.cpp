#include <streamlogger/category.h>
#include <streamlogger/configurator.h>

int main() {
    using namespace streamlogger;
    configure("../../../log.tests.ini");

    error("") << "Hello bitches" << " " << 42;

    info("wrapper") << "C'mon meeen" << 5;
}
