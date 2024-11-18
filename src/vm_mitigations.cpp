#include "vm_mitigations.h"
#include <iostream>
#include <map>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

const std::string kernel_module_dir = " ../kernel_modules/";
const std::string make = "make -C ";
const std::string insmod = "sudo insmod ";
const std::string rmmod = "sudo rmmod ";
const std::string ko_extension = ".ko";

bool buildModules(){
    int result = std::system( (make + kernel_module_dir).c_str() );
    if(result != 0)
    {
        std::cerr << "Error: Failed to build kernel module." << std::endl;
        return false;
    }
    return true;
}

void applyMitigations(const std::map<std::string, bool> test_results) {
    std::cout <<"\nBuilding kernel modules..."<<std::endl;
    buildModules();
    std::cout << "\nApplying mitigation techniques for detected artifacts...\n";

    // Check each test and apply mitigation if it was detected
    for (const auto& [testName, detected] : test_results) {
        if (detected) {
            if(testName == "dmi") mitigateDMI();
        /** 
            if (testName == "io") mitigateIODevices();
            else if (testName == "cpu") mitigateCPU();
            else if (testName == "cpuid-vendor") mitigateVendorID();
            else if (testName == "dmi") mitigateDMI();
            else if (testName == "mac") mitigateMAC();
            else if (testName == "pci") mitigatePCI();
            else if (testName == "timing") mitigateTiming();
            else if (testName == "desc-tables") mitigateDescriptorTables();
            else if (testName == "acpi") mitigateACPI();
            else if (testName == "lscpu") mitigateLSCPU();
            else if (testName == "usb") mitigateUSB();
            else if (testName == "env") mitigateEnvVars();
            else cout << "No mitigation available for test: " << testName << endl;
        */
        }
    }
    std::cout << "\nMitigations applied!";
}



bool mitigateDMI()
{
    std::cout << "=====Mitigating DMI Detection=====\n  Loading kernel module..." << std::endl;
    const std::string module_name = "dmi_module.ko";

    std::string load_command = insmod + kernel_module_dir + module_name;
    int insmod_result = std::system(load_command.c_str());

    if (insmod_result != 0)
    {
        std::cerr<<"Error: Failed to load kernel module" << std::endl;
        return false;
    }


    return true;
}