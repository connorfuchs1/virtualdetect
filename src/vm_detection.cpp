#include "vm_detection.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

//All of our tests
const int NUMTESTS = 3;
const char* tests[3] =
{
    "io", "cpu", "cpuid-query"
};

// Function to display help message
void displayHelp() {
    cout << "Usage: vm_detection [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -h           Display this help message" << endl;
    cout << "  -a           Run all tests" << endl;
    cout << "  -t <test>    Run individual test (e.g., io, cpu)" << endl;
}

// Function to run all tests
int runAllTests(OS_TYPE OS) {
    int totalTests = 0;
    int detected = 0;

    if (checkIODevices(OS)) {
        detected++;
    }
    totalTests++;

    if (checkCPUFeatures(OS)) {
        detected++;
    }
    totalTests++;

    if (checkCPUID(OS)){
        detected++;
    }
    totalTests++;

    // Add more tests here as needed

    cout << "\n\t   ==============================================" << endl;
    cout << "\tResult: " << detected << " of " << totalTests << " tests found virtualization artifacts" << endl;

    return detected > 0;
}

// Function to run individual tests
int runIndividualTest(OS_TYPE OS, const std::string& testName) 
{
    if (testName == "io") 
    {
        return checkIODevices(OS);
    } 
    else if (testName == "cpu") 
    {
        return checkCPUFeatures(OS);
    }
    else if (testName == "cpuid-query"){
        return checkCPUID(OS);
    } 
    else 
    {
        cout << "Unknown test name: " << testName << endl;
        return -1;
    }
}

bool checkCPUID(OS_TYPE OS){
    cout << "===== Checking CPUID =====" << endl;
    if (OS == OS_LINUX)
    {
        unsigned int eax, ebx, ecx, edx;
        char hyper_vendor[13];

        eax = 0x40000000;

        __asm__ __volatile__(
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
        );

        memcpy(hyper_vendor + 0, &ebx, 4);
        memcpy(hyper_vendor + 4, &ecx, 4);
        memcpy(hyper_vendor + 8, &edx, 4);
        hyper_vendor[12] = '\0';

        if (strlen(hyper_vendor) > 0) 
        {
        printf("Hypervisor Vendor ID: %s\n", hyper_vendor);
        } 

        else 
        {
        printf("No Hypervisor Vendor ID found.\n");
        return false;
        }

    }
    return true;
}

// CPU Features Test
bool checkCPUFeatures(OS_TYPE OS) 
{
    cout << "===== Checking CPU Features =====" << endl;

    if (OS == OS_LINUX) 
    {
        unsigned int eax, ecx;
        eax = 1; // Processor Info and Feature Bits

        __asm__ __volatile__(
            "cpuid"
            : "=c"(ecx)
            : "a"(eax)
            : "%ebx", "%edx"
        );

        if (ecx & (1 << 31)) {
            printf("Hypervisor bit is set. Running inside a virtual machine.\n");
        } else {
            printf("Hypervisor bit is not set. Likely running on physical hardware.\n");
            return false;
        }

        return true;
    } 
    
    else if (OS == OS_WINDOWS) 
    {
        // Implement Windows-specific CPU features check
        return false;
    } 
    
    else 
    {
        cout << "Unsupported OS for CPU features check." << endl;
        return false;
    }
}

// IO Devices Test
bool checkIODevices(OS_TYPE OS) 
{
    cout << "===== Checking IO Devices =====" << endl;
    vector<string> vm_signatures = {"VMware", "VirtualBox", "QEMU", "Microsoft"};

    if (OS == OS_LINUX) 
    {
        ifstream io_device_file("/proc/bus/input/devices");
        if (!io_device_file.is_open()) 
        {
            cerr << "Error opening /proc/bus/input/devices" << endl;
            return false;
        }

        string line;
        bool detected = false;
        while (getline(io_device_file, line)) 
        {
            for (const auto& signature : vm_signatures) 
            {
                if (line.find(signature) != string::npos) 
                {
                    cout << "Detected VM Vendor in IO devices: " << line << endl;
                    detected = true;
                }
            }
        }
        return detected;


    } 
    else if (OS == OS_WINDOWS) 
    {
        // Implement Windows-specific IO devices check
        return false;
    } 
    else 
    {
        cout << "Unsupported OS for IO device check." << endl;
        return false;
    }
}



