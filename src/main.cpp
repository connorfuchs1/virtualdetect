#include "vm_detection.h"
#include <iostream>
#include <unistd.h> // For getopt on Unix/Linux systems

using namespace std;


int main(int argc, char* argv[]) {
    // Detect and display the operating system
    if (WINDOWS) 
    {
        cout << "Running Windows" << endl;
        OS = OS_WINDOWS;
    }
     else if (LINUX) {
        cout << "Linux system detected.. running linux checks" << endl;
        OS = OS_LINUX;
    } 
    else 
    {
        cout << "Unknown OS" << endl;
    }

    // Detect and display the architecture
    if (X86_64) 
    {
        ARCH = ARCH_X86_64;
    } else if (X86) 
    {
        ARCH = ARCH_X86;
    } else if (ARM64) 
    {
        ARCH = ARCH_ARM64;
    } else if (ARM) 
    {
        ARCH = ARCH_ARM;
    } else 
    {
        cout << "Unknown architecture" << endl;
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
        runAllTests();
    } else if (!testName.empty()) {
        runIndividualTest(testName);
    } else {
        displayHelp();
        return -1;
    }

    return 0;
}
