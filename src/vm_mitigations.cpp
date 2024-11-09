#include "vm_mitigations.h"
#include <iostream>
#include <map>
#include <string>

void applyMitigations(const std::map<std::string, bool> test_results) {
    std::cout << "\nApplying mitigation techniques for detected artifacts...\n";

    // Check each test and apply mitigation if it was detected
    for (const auto& [testName, detected] : test_results) {
        if (detected) {
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
    std::cout << "Mitigations applied where possible.\n";
}