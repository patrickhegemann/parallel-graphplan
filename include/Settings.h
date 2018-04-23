#ifndef _PARALLELGP_SETTINGS_H
#define _PARALLELGP_SETTINGS_H

#include <string>
#include <stdlib.h>

#include "Settings.h"
#include "ParameterProcessor.h"
#include "Logger.h"


#define HORIZON_LINEAR 1
#define HORIZON_EXPONENTIAL 2


class Settings {
    private:
        int verbosityLevel;
        const char *inputFile;
        int dumpPlanningGraph = 0;

        std::string plannerName;
        int threadCount;

        int horizonType;
        double horizonFactor;

        int layerPackSize;

    public:
        Settings();
        Settings(int argc, char **argv) {
            ParameterProcessor pp;
            pp.init(argc, argv);

            inputFile = pp.getFilename();
            verbosityLevel = pp.getIntParam("v", 0);
            dumpPlanningGraph = pp.isSet("dump");

            plannerName = pp.getParam("p", "sppsat");
            threadCount = pp.getIntParam("t", 2);

            // Set the type of horizon accordingly as an int to save runtime
            std::string ht = pp.getParam("h", "lin");
            if (ht == "lin") {
                horizonType = HORIZON_LINEAR;
            } else if (ht == "exp") {
                horizonType = HORIZON_EXPONENTIAL;
            }

            // Cast parameter to a double
            std::string hf = pp.getParam("hf", "4");
            horizonFactor = atof(hf.c_str());

            layerPackSize = pp.getIntParam("lps", 4);

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

        std::string getPlannerName() {
            return plannerName;
        }

        int getThreadCount() {
            return threadCount;
        }

        int getHorizonType() {
            return horizonType;
        }

        double getHorizonFactor() {
            return horizonFactor;
        }

        int getLayerPackSize() {
            return layerPackSize;
        }
};

extern Settings *settings;

#endif /* _PARALLELGP_SETTINGS_H */

