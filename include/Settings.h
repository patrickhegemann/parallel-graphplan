#ifndef SETTINGS_H
#define SETTINGS_H


class Settings {
    private:
        int verbosityLevel;
        const char *inputFile;

    public:
	    Settings(int argc, char **argv);

        int getVerbosityLevel();
        const char* getInputFile();
};

#endif

