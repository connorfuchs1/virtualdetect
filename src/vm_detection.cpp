#include "vm_detection.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

//================================Globals===============================

//common VM signatures
vector<string> vm_signatures = 
{
    "VMware", "VirtualBox", "QEMU", 
    "KVM", "Microsoft Corporation", 
    "Parallels", "Xen", "Bochs", "BHYVE"
};

//number of tests
const int NUMTESTS = 4;
const char* tests[4] =
{
    "io", "cpu-hypervisorbit", "cpuid-vendor", "dmi"
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

    if (checkHypervisorBit(OS)) {
        detected++;
    }
    totalTests++;

    if (checkVendorID(OS)){
        detected++;
    }
    totalTests++;
    
    if (checkDMI(OS)){
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
        return checkHypervisorBit(OS);
    }
    else if (testName == "cpuid-vendor"){
        return checkVendorID(OS);
    }
    else if (testName == "dmi"){
        return checkDMI(OS);
    } 
    else 
    {
        cout << "Unknown test name: " << testName << endl;
        return -1;
    }
}
//======================================TESTS===========================================


/**
    Test to check for VM signatures in DMI fields
 */
bool checkDMI(OS_TYPE OS){
    cout<<"===== Checking DMI Fields =====" <<endl;
    //Checking for Linux Systems
    if (OS == OS_LINUX)
    {
        //Typical DMI paths on linux
        const std::vector<std::string> dmi_paths = {
        "/sys/class/dmi/id/sys_vendor",
        "/sys/class/dmi/id/product_name",
        "/sys/class/dmi/id/product_version"
        };

        bool detected = false;
        //Check each DMI field for any VM signatures
        for (const auto& path : dmi_paths)
        {
            ifstream file(path);
            string value;

            if(file.is_open() && getline(file, value))
            {
                for(const auto& signature : vm_signatures)
                {
                    if (value.find(signature) != std::string::npos) 
                    {
                        detected = true; //We found a signature.
                        cout<<"Signature found :"<<"\""<<signature<<"\"" <<" in DMI field: "<<path<<endl;
                    }
                }
            }
            
        }
        return detected;
    }

    if(OS == OS_WINDOWS)
    {

    }

    if (OS == OS_MAC)
    {

    }

    return false;
}

/**
    Test for checking EAX=0x40000000 for a Vendor ID string via CPUID.
 */
bool checkVendorID(OS_TYPE OS){
    cout << "===== Checking Hypervisor Vendor ID =====" << endl;
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

/**
    Test to check whether the hypervisor bit is set.
 */
bool checkHypervisorBit(OS_TYPE OS) 
{
    cout << "===== Checking CPU Hypervisor Bit =====" << endl;

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
            printf("Hypervisor bit is set.\n");
        } else {
            printf("Hypervisor bit is not set.\n");
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

/**
    Test to check if IO Devices have VM signatures attached.
 */
bool checkIODevices(OS_TYPE OS) 
{
    cout << "===== Checking IO Devices =====" << endl;

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



