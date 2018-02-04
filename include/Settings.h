#ifndef _PARALLELGP_SETTINGS_H
#define _PARALLELGP_SETTINGS_H


class Settings {
    private:
        int verbosityLevel;
        const char *inputFile;
        int dumpPlanningGraph = 0;

    public:
        Settings();
	    Settings(int argc, char **argv);

        int getVerbosityLevel();
        const char* getInputFile();
        int getDumpPlanningGraph();
};

extern Settings *settings;

#endif /* _PARALLELGP_SETTINGS_H */

