#ifndef _PARALLELGP_SETTINGS_H
#define _PARALLELGP_SETTINGS_H

#include "Settings.h"
#include "ParameterProcessor.h"
#include "Logger.h"


class Settings {
    private:
        int verbosityLevel;
        const char *inputFile;
        int dumpPlanningGraph = 0;

    public:
        Settings();
        Settings(int argc, char **argv) {
            ParameterProcessor pp;
            pp.init(argc, argv);

            inputFile = pp.getFilename();
            verbosityLevel = pp.getIntParam("v", 0);
            dumpPlanningGraph = pp.isSet("dump");

            log(0, "Parameters: ");
            pp.printParams();
        }

        int getVerbosityLevel() {
            return verbosityLevel;
        }

        const char* getInputFile() {
            return inputFile;
        }

        int getDumpPlanningGraph() {
            return dumpPlanningGraph;
        }
};

extern Settings *settings;

#endif /* _PARALLELGP_SETTINGS_H */

