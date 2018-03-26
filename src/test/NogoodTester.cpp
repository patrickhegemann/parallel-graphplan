#include <list>

#include "Planner.h"
#include "Settings.h"
#include "common.h"



Settings* settings;


void test(int number, bool success) {
    if (success) {
        log(0, "Test %d successful\n", number);
    } else {
        log(0, "Test %d failed\n", number);
    }
}


int main(int argc, char *argv[]) {
    settings = new Settings(argc, (char**)argv);
    setVerbosityLevel(settings->getVerbosityLevel());

    Planner p(NULL);

    std::list<Proposition> firstNogood = {Proposition(1, 1), Proposition(0, 0), Proposition(3, 3)};
    p.addNogood(1, firstNogood);

    p.dumpNogoods();

    test(1, p.isNogood(1, firstNogood));

    std::list<Proposition> secondNogood = {Proposition(1, 1), Proposition(0, 0), Proposition(0, 0), Proposition(3, 3)};
    test(2, p.isNogood(1, secondNogood));
    test(3, p.isNogood(1, firstNogood));

    return 0;
}

