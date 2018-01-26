#include "Settings.h"
#include "ParameterProcessor.h"
#include "Logger.h"

Settings::Settings(int argc, char **argv) {
	ParameterProcessor pp;
	pp.init(argc, argv);

    inputFile = pp.getFilename();
    verbosityLevel = pp.getIntParam("v", 0);

    log(0, "Parameters: ");
    pp.printParams();
}


int Settings::getVerbosityLevel() {
    return verbosityLevel;
}

const char* Settings::getInputFile() {
    return inputFile;
}

