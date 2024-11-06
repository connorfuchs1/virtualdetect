#ifndef VM_DETECTION_H
#define VM_DETECTION_H

#include <string> 

// Architecture Detection Macros
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
    #define X86_64 1
#else
    #define X86_64 0
#endif

#if defined(__i386) || defined(_M_IX86)
    #define X86 1
#else
    #define X86 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
    #define ARM64 1
#else
    #define ARM64 0
#endif

#if defined(__arm__) || defined(_M_ARM)
    #define ARM 1
#else
    #define ARM 0
#endif

// OS Detection Macros (Optional, if needed)
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

enum OS_TYPE {
    OS_UNKNOWN = -1,
    OS_WINDOWS,
    OS_LINUX,
    OS_MAC
};

enum ARCH_TYPE {
    ARCH_UNKNOWN = -1,
    ARCH_X86,
    ARCH_X86_64,
    ARCH_I386,
    ARCH_ARM,
    ARCH_ARM64
};

extern OS_TYPE OS;
extern ARCH_TYPE ARCH;

// Function declarations
void displayHelp();
int runAllTests();
int runIndividualTest(const std::string& testName);

//individual tests
bool checkIODevices();
bool checkHypervisorBit();
bool checkVendorID();
bool checkDMI();
bool checkMAC();
bool checkPCI();

// Add additional test function declarations here...

#endif // VM_DETECTION_H
