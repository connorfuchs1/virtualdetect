#include "vm_detection.h"
#include <iostream>
#include <unistd.h> // For getopt on Unix/Linux systems

using namespace std;

// OS Detection Macros
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER)
    #define WINDOWS 1
#else
    #define WINDOWS 0
#endif

#if defined(__linux__) || defined(__GNUC__)
    #define LINUX 1
#else
    #define LINUX 0
#endif

int main(int argc, char* argv[]) {
    OS_TYPE OS = OS_UNKNOWN;
    if (WINDOWS) {
        cout << "Running Windows" << endl;
        OS = OS_WINDOWS;
    } else if (LINUX) {
        cout << "Linux system detected.. running linux checks" << endl;
        OS = OS_LINUX;
    } else {
        cout << "Unknown OS" << endl;
    }

    // Command-line options
    int option;
    bool runAll = false;
    string testName;

    while ((option = getopt(argc, argv, "hat:")) != -1) {
        switch (option) {
            case 'h':
                displayHelp();
                return 0;
            case 'a':
                runAll = true;
                break;
            case 't':
                testName = optarg;
                break;
            default:
                displayHelp();
                return -1;
        }
    }

    if (runAll) {
        runAllTests(OS);
    } else if (!testName.empty()) {
        runIndividualTest(OS, testName);
    } else {
        displayHelp();
        return -1;
    }

    return 0;
}
