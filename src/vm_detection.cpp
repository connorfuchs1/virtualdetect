#include "vm_detection.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <string>
#include <functional>
#include <map>

#ifdef __x86_64__
    #include <cpuid.h>
#endif    
using namespace std;

//================================Globals===============================

//List of Common VM signatures
vector<string> vm_signatures = 
{
    "VMware", "VirtualBox", "QEMU", 
    "KVM", "Microsoft Corporation", 
    "Parallels", "Xen", "Bochs", "BHYVE"
};

// List of common VM MAC address prefixes (need to get more accurate)
const std::vector<std::string> vm_mac_prefixes = {
    "00:05:69",  // VMware
    "00:0C:29",  // VMware
    "00:1C:14",  // VMware
    "00:50:56",  // VMware
    "08:00:27",  // VirtualBox
    "52:54:00",  // QEMU/KVM
    "00:15:5D",  // Microsoft Hyper-V
    "00:16:3E",  // Xen
    "00:1A:4A"   // Parallels
};

// Known PCI vendor and device IDs for virtual devices
std::vector<std::string> virtual_device_ids = {
    "15ad:0740", // VMware
    "15ad:0405", // VMware
    "15ad:07a0", // VMware
    "1234:1111", // QEMU
    "1af4:1000", // Virtio (QEMU)
    "80ee:beef", // VirtualBox
    "10de:1db6", // NVIDIA vGPU for virtual environments
    "5853:0001", // Xen
    "5853:0002", // Xen
};

//Map for storing all of our tests
static const std::map<std::string, std::function<bool(OS_TYPE)>> tests = {
        {"io", checkIODevices},
        {"cpu", checkHypervisorBit},
        {"cpuid-vendor", checkVendorID},
        {"dmi", checkDMI},
        {"mac", checkMAC},
        {"pci", checkPCI}
        // Add more test mappings here as needed
    };

//number of tests
const int NUMTESTS = tests.size();

// Function to display help message
void displayHelp() 
{
    cout << "Usage: vm_detection [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -h           Display this help message" << endl;
    cout << "  -a           Run all tests" << endl;
    cout << "  -t <test>    Run individual test (e.g., io, cpu)" << endl;
}

// Function to run all tests
int runAllTests(OS_TYPE OS) 
{
    int totalTests = tests.size();
    int detected = 0;

    // Run all tests
    for (const auto& [testName, testFunction] : tests) 
    {
        if (testFunction(OS)) {
            detected++;
        }
    }


    cout << "\n\t===================================================" << endl;
    cout << "\tResult: " << detected << " of " << totalTests << " tests found virtualization artifacts" << endl;
    cout << "\tChance virtualization detected: "<< ((float)detected/totalTests)*100 << "%" <<endl;
    return detected > 0;
}

// Function to run individual tests
int runIndividualTest(OS_TYPE OS, const std::string& testName) 
{
    // Look up the test function by name
    auto it = tests.find(testName);
    if (it != tests.end()) 
    {
        // Run the test if it exists
        return it->second(OS);
    } 
    else 
    {
        cout << "Unknown test name: " << testName << endl;
        return -1;
    }
}

//======================================TESTS===========================================

/**
    Test to check PCI vendor and device ID's for virtual Devices
 */
bool checkPCI(OS_TYPE OS){
    cout<<"===== Checking for virtualized PCI devices =====" << endl;
    if (OS == OS_LINUX)
    {
        

    }
    if (OS == OS_WINDOWS)
    {


    }
    if (OS == OS_MAC)
    {


    }
    return false;
}

/**
    Test to check our MAC ADDRESS for common VM address prefixes.
 */
bool checkMAC(OS_TYPE OS){
    cout<<"===== Checking Common MAC Addresses ====="<< endl;
    bool detected = false;

    if(OS == OS_LINUX)
    {
        ifstream netDev("/proc/net/dev");
        if (!netDev.is_open()) 
        {
        cerr << "Cannot open /proc/net/dev" << endl;
        return false;
        }
        string line;
        // Skip the first two header lines
        getline(netDev, line);
        getline(netDev, line);
        
        while(getline(netDev, line))
        {
            istringstream iss(line);
            string iface;

            //get the interface name
            if(getline(iss, iface, ':'))
            {
                iface.erase(0, iface.find_first_not_of(" \t"));
                iface.erase(iface.find_last_not_of(" \t") + 1);

                //now we construct path to the MAC address file:
                string macPath = "/sys/class/net/" + iface + "/address";
                ifstream macFile(macPath);

                if(macFile.is_open())
                {
                    string mac;
                    getline(macFile, mac);
                    macFile.close();
                    std::transform(mac.begin(),mac.end(),mac.begin(), ::toupper);
                    //now we check this mac address to see if it matches any known prefix addys
                    for(const auto& prefix : vm_mac_prefixes)
                    {
                        if (mac.find(prefix) == 0 )
                        {
                            cout << "Virtual NIC prefix detected: " << iface 
                            << " with MAC " << mac << endl;
                            detected = true;
                        }
                    }

                }
            }
        }

    }
    if (OS == OS_WINDOWS){


    }

    if (OS == OS_MAC){


    }
    return detected;
}


/**
    Test to check for VM signatures in DMI fields
 */
bool checkDMI(OS_TYPE OS){
    cout<<"===== Checking DMI Fields =====" <<endl;
    //Checking for Linux Systems
    if (OS == OS_LINUX)
    {
        //Typical DMI paths on linux
        const vector<string> dmi_paths = {
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
    std::cout << "===== Checking Hypervisor Vendor ID =====" << std::endl;

#ifdef __x86_64__
    // x86/x86-64 specific code using cpuid
    if (OS == OS_LINUX) {
        unsigned int eax = 0x40000000;
        unsigned int ebx, ecx, edx;
        char hyper_vendor[13];

        if (__get_cpuid(eax, &eax, &ebx, &ecx, &edx)) {
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
        } else {
            printf("cpuid instruction not supported.\n");
            return false;
        }
    }

#elif defined(__aarch64__) || defined(__arm__)
    // ARM-specific virtualization detection
    if (OS == OS_LINUX) {
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (!cpuinfo.is_open()) {
            std::cerr << "Error opening /proc/cpuinfo" << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(cpuinfo, line)) {
            // Look for hypervisor indicators in /proc/cpuinfo
            if (line.find("Hypervisor") != std::string::npos || line.find("VM") != std::string::npos) {
                std::cout << "Potential hypervisor detected in /proc/cpuinfo: " << line << std::endl;
                return true;
            }
        }
        cpuinfo.close();
        std::cout << "No hypervisor vendor detected in /proc/cpuinfo." << std::endl;
        return false;
    }
#else
    std::cout << "Unsupported architecture for hypervisor detection." << std::endl;
    return false;
#endif
    return true;
}
/**
    Test to check whether the hypervisor bit is set.
 */
bool checkHypervisorBit(OS_TYPE OS) 
{
    cout << "===== Checking CPU Hypervisor Bit =====" << endl;
#ifdef __x86_64__
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

#elif defined(__aarch64__) || defined(__arm__)
    if (OS == OS_LINUX)
    {
        cout<<"On arm based processor, implement check hypervisor bit"<<endl;
    }
#else
    std::cout << "Unsupported architecture for hypervisor detection." << std::endl;
    return false;
#endif
    return true;

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



