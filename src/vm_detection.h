#ifndef VM_DETECTION_H
#define VM_DETECTION_H

enum OS_TYPE {
    OS_UNKNOWN = -1,
    OS_WINDOWS,
    OS_LINUX,
    OS_MAC
};

// Function declarations
void displayHelp();
int runAllTests(OS_TYPE OS);
int runIndividualTest(OS_TYPE OS, const std::string& testName);

//individual tests
bool checkIODevices(OS_TYPE OS);
bool checkHypervisorBit(OS_TYPE OS);
bool checkVendorID(OS_TYPE OS);
bool checkDMI(OS_TYPE OS);
bool checkMAC(OS_TYPE OS);

// Add additional test function declarations here...

#endif // VM_DETECTION_H
