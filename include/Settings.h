#ifndef _PARALLELGP_SETTINGS_H
#define _PARALLELGP_SETTINGS_H


class Settings {
    private:
        int verbosityLevel;
        const char *inputFile;

    public:
	    Settings(int argc, char **argv);

        int getVerbosityLevel();
        const char* getInputFile();
};

#endif /* _PARALLELGP_SETTINGS_H */

