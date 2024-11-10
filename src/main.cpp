#include "vm_detection.h"
#include "vm_mitigations.h"  // Include the mitigations header
#include <iostream>
#include <unistd.h> // For getopt on Unix/Linux systems

using namespace std;

int main(int argc, char* argv[]) {
    bool runAll = false;
    string testName;

    // Detect and display the OS and Architecture
    if (LINUX) {
        OS = OS_LINUX;
    } 
    else if (WINDOWS) {
        OS = OS_WINDOWS;
    } else 
    {
        cout << "Unknown OS" << endl;
        return -1;
    }

    if (X86_64) ARCH = ARCH_X86_64;
    else if (X86) ARCH = ARCH_X86;
    else if (ARM64) ARCH = ARCH_ARM64;
    else if (ARM) ARCH = ARCH_ARM;
    else cout << "Unknown architecture" << endl;

    int option;
    while ((option = getopt(argc, argv, "hat:")) != -1) 
    {
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

    bool exitProgram = false;
    while (!exitProgram) {
        map<string, bool> test_results;

        if (runAll) {
            test_results = runAllTests();  // Store results in the map
        } else if (!testName.empty()) {
            runIndividualTest(testName);
        } else {
            displayHelp();
            return -1;
        }

        // Ask user if they want to apply mitigation techniques
        char userChoice;
        cout << "\nWould you like to apply mitigation techniques for detected artifacts? (y/n): ";
        cin >> userChoice;

        if (userChoice == 'y' || userChoice == 'Y') {
            applyMitigations(test_results);  // Call mitigation function with results
        }

        // Ask if they want to exit or re-run
        cout << "Would you like to re-run tests, or exit? (r/e): ";
        cin >> userChoice;
        if (userChoice == 'e' || userChoice == 'E') {
            exitProgram = true;
        } else {
            cout << "Re-running tests..." << endl;
        }
    }
    return 0;
}
