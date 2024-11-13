#include "vm_detection.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <functional>
#include <pci/pci.h>
#include <regex>
#include <csignal>
#include <csetjmp>
#include <cstdint>
#include <numeric>

#ifdef __x86_64__
    #include <cpuid.h>
#endif    

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#endif

using namespace std;

//================================Globals===============================
OS_TYPE OS = OS_UNKNOWN;
ARCH_TYPE ARCH = ARCH_UNKNOWN;

//List of Common VM signatures
vector<string> vm_signatures = {
    "VMware", "VirtualBox", "VBOX", "QEMU",
    "KVM", "Microsoft Corporation", "Hyper-V",
    "Parallels", "Bochs", "BHYVE",
    "HVM domU", "innotek GmbH", "QEM", "VRT"
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
    "0x15ad:0x0740", // VMware
    "0x15ad:0x0405", // VMware
    "0x15ad:0x07a0", // VMware
    "1234:1111", // QEMU
    "1af4:1000", // Virtio (QEMU)
    "80ee:beef", // VirtualBox
    "10de:1db6", // NVIDIA vGPU for virtual environments
    "5853:0001", // Xen
    "5853:0002", // Xen
};

//Map for storing all of our tests
static const std::map<std::string, std::function<bool()>> tests = {
        {"io", checkIODevices},
        {"cpu", checkHypervisorBit},
        {"cpuid-vendor", checkVendorID},
        {"dmi", checkDMI},
        {"mac", checkMAC},
        {"pci", checkPCI},
        {"timing", checkTiming},
        {"desc-tables", checkDescriptorTables},
        {"acpi", checkACPI},
        {"lscpu", checklscpu},
        {"usb", checkUSBDevices},
        {"env", checkEnvVars}


        // Add more test mappings here as needed
    };

// Test Result map
std::map<std::string, bool> test_results;

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
std::map<std::string, bool> runAllTests()
{
    int totalTests = tests.size();
    int detected = 0;
    cout << ARCH << endl;

    // Run all tests and store results
    for (const auto& [testName, testFunction] : tests) 
    {
        bool result = testFunction();  // Run the test
        test_results[testName] = result;  // Store the result in test_results map
        if (result) {
            detected++;
        }
    }

    // Display results in a formatted box
    cout << "\n\t╔══════════════════════════════════════════════════════════════════╗" << endl;
    cout << "\t║                  Virtualization Detection Summary                ║" << endl;
    cout << "\t╠══════════════════════════════════════════════════════════════════╣" << endl;
    cout << "\t║ Result: " << detected << " of " << totalTests << " tests found virtualization artifacts.\t\t   ║" << endl;
    cout << "\t║ Chance virtualization detected: " << std::fixed << std::setprecision(2) 
         << ((float)detected/totalTests)*100 << "%                           ║" << endl;
    cout << "\t╠══════════════════════════════════════════════════════════════════╣" << endl;

    displayResults(test_results);

    cout << "\t╚══════════════════════════════════════════════════════════════════╝" << endl;

    return test_results;
}

// Function to run individual tests
int runIndividualTest(const std::string& testName) 
{
    // Look up the test function by name
    auto it = tests.find(testName);
    if (it != tests.end()) 
    {
        // Run the test if it exists
        return it->second();
    } 
    else 
    {
        cout << "Unknown test name: " << testName << endl;
        return -1;
    }
}

// Function to display test results in box format
void displayResults(const std::map<std::string, bool> test_results) {
    // Calculate max test name width
    int maxTestNameWidth = 0;
    for (const auto& [testName, _] : test_results) {
        maxTestNameWidth = std::max(maxTestNameWidth, static_cast<int>(testName.size()));
    }

    // Set width for result text and padding
    int maxResultWidth = std::max(static_cast<int>(std::string("Detected").size()), static_cast<int>(std::string("Not Detected").size()));
    int totalWidth = maxTestNameWidth + maxResultWidth + 13; // Adjust total width based on max widths

    for (const auto& [testName, result] : test_results) {
        std::string resultText = result ? "Detected" : "Not Detected";
        
        // Print line with padding to align right side
        cout << "\t║ Test: " << std::left << std::setw(maxTestNameWidth) << testName
             << " │ " << std::setw(maxResultWidth) << resultText
             << std::string(totalWidth - maxTestNameWidth , ' ')  // Dynamic right-side padding
             << "\t   ║" << endl;
    }
}
//======================================TESTS===========================================

/**
    Function to check for evidence of any virtualization signatusres in environment variables
 */
bool checkEnvVars() {
    std::cout << "===== Checking Environment Variables for Virtualization Signatures =====" << std::endl;

    // Run printenv and capture output
    FILE* pipe = popen("printenv", "r");
    if (!pipe) {
        std::cerr << "Failed to run printenv command." << std::endl;
        return false;
    }

    char buffer[128];
    bool detected = false;

    // Read printenv output line-by-line
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);

        // Check each line for any known VM signatures
        for (const auto& signature : vm_signatures) 
        {
            if (line.find(signature) != std::string::npos) 
            {
                std::cout << "Virtualization signature found in environment variable: " << line;
                detected = true;
                break;
            }
        }
    }
    pclose(pipe);

    if (!detected) 
    {
        std::cout << "No virtualization indicators found in environment variables." << std::endl;
    }
    return detected;
}


/**
    Function to check for virtualization artifacts among listed usb devices.
 */
bool checkUSBDevices() {
    std::cout << "===== Checking USB Devices for Virtualization Artifacts =====" << std::endl;
    char buffer[128];
    bool detected = false;

    // Execute `lsusb` command and capture output
    FILE* pipe = popen("lsusb", "r");
    if (!pipe) {
        std::cerr << "Failed to run lsusb command." << std::endl;
        return false;
    }

    // Read lsusb output line-by-line
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);

        // Check each line for any known VM signatures
        for (const auto& signature : vm_signatures) 
        {
            if (line.find(signature) != std::string::npos) 
            {
                std::cout << "Virtualization signature found in USB device: \n" << line;
                detected = true;
                break;
            }
        }
    }
    pclose(pipe);

    if (!detected) {
        std::cout << "No virtualization indicators found in USB devices." << std::endl;
    }
    return detected;
}

/**
    Function to check lscpu output to check for virtualization signatures
 */
bool checklscpu() {
    std::cout << "===== Checking lscpu Output for VM Signatures =====" << std::endl;
    bool detected = false;
    // Run lscpu command and redirect output to a file
    std::system("lscpu > lscpu_output.txt");
    std::ifstream lscpu_file("lscpu_output.txt");
    if (!lscpu_file.is_open()) 
    {
        std::cerr << "Could not open lscpu output file." << std::endl;
        return detected;
    }
    std::string line;
    while (std::getline(lscpu_file, line)) 
    {
        // Check each line for any VM signature
        for (const auto& signature : vm_signatures) 
        {
            //KVM appears in cpu vuln mitigations 
            if(signature != "KVM"){
 
                if (line.find(signature) != std::string::npos) 
                {
                    std::cout << "Virtualization signature found in lscpu output: \n" << line << std::endl;
                    detected = true;
                    break;
                }
            }
        }
    }
    lscpu_file.close();
    if (detected) 
    {
        std::cout << "Virtualization detected based on VM signatures in lscpu output." << std::endl;
    } else 
    {
        std::cout << "No virtualization signatures found in lscpu output." << std::endl;
    }
    return detected;
}


/**
    Function to check ACPI tables for virtualization artifacts
 */
bool checkACPI() {
    std::cout << "===== Checking ACPI Tables =====" << std::endl;
    bool detected = false;

    // Construct the regex pattern by joining the signatures
    std::string pattern = "(";
    for (size_t i = 0; i < vm_signatures.size(); ++i) 
    {
        pattern += vm_signatures[i];
        if (i != vm_signatures.size() - 1) {
            pattern += "|";
        }
    }
    pattern += ")";

    std::regex signature_regex(pattern, std::regex_constants::icase);

    // Temporary file to store ACPI dump
    const std::string acpi_dump_file = "/tmp/acpi_tables.txt";

    // Execute acpidump command and redirect output to the file
    int ret = system(("sudo acpidump > " + acpi_dump_file).c_str());
    if (ret != 0) 
    {
        std::cerr << "Failed to execute acpidump. Ensure you have the necessary permissions." << std::endl;
        return false;
    }

    // Open the dumped ACPI tables file
    std::ifstream acpi_file(acpi_dump_file);
    if (!acpi_file.is_open()) 
    {
        std::cerr << "Failed to open ACPI dump file." << std::endl;
        return false;
    }

    std::string line;
    std::smatch match;
    int linenum = 0;
    // Read the file line by line
    while (std::getline(acpi_file, line)) 
    {
        // Search for any of the virtualization signatures
        if (std::regex_search(line, match, signature_regex)) 
        {
            std::cout << "Virtualization signature found in ACPI table: " << match.str(1) << "\n\t Line: " << linenum << std::endl;
            detected = true;
        }
        linenum++;
    }

    acpi_file.close();

    // Remove the temporary file
    std::remove(acpi_dump_file.c_str());

    return detected;
}




// Jump buffer for non-local goto in signal handler
jmp_buf buf;

/**
 * Signal handler for catching segmentation faults.
 * Uses longjmp to return to a safe execution point.
 */
void signal_handler(int signum) {
    int x = signum;
    cout<<x<<endl;
    longjmp(buf, 1); // Jump back to setjmp point with return value 1
}

/**
 * Checks the base addresses of the GDT and IDT.
 * Detects possible virtualization by analyzing these addresses.
 */
bool checkDescriptorTables() {
    std::cout << "===== Checking Descriptor Tables =====" << std::endl;
    bool virtualization_detected = false;


    if(OS == OS_LINUX){

    
    #if defined(__x86_64__)
        // Set up signal handler for SIGSEGV to catch segmentation faults
        std::signal(SIGSEGV, signal_handler);

        // Buffers to store GDTR and IDTR contents
        struct {
            uint16_t limit;
            uint64_t base;
        } __attribute__((packed)) gdtr, idtr;


        // Use setjmp/longjmp for exception handling
        if (setjmp(buf) == 0) {
            // Attempt to execute SGDT and SIDT instructions
            asm volatile("sgdt %0" : "=m"(gdtr));
            asm volatile("sidt %0" : "=m"(idtr));

            // If no exception occurred, analyze the base addresses
            std::cout << "SGDT and SIDT executed successfully." << std::endl;

            std::cout << "GDTR Base: 0x" << std::hex << gdtr.base << std::endl;
            std::cout << "IDTR Base: 0x" << std::hex << idtr.base << std::endl;

            //reset to decimal format
            std::cout << std::dec;

            // Check if base addresses are in user space (unexpected)
            if (gdtr.base < 0xFFFF800000000000) {
                std::cout << "GDTR base address is in user space (unexpected). Possible virtualization detected." << std::endl;
                virtualization_detected = true;
            } else {
                std::cout << "GDTR base address is in kernel space (expected)." << std::endl;
            }

            if (idtr.base < 0xFFFF800000000000) {
                std::cout << "IDTR base address is in user space (unexpected). Possible virtualization detected." << std::endl;
                virtualization_detected = true;
            } else {
                std::cout << "IDTR base address is in kernel space (expected)." << std::endl;
            }

            // Additional checks for known virtualization signatures
            // For example, checking for specific base addresses used by VMware
            if (idtr.base == 0xfff82000 || gdtr.base == 0xfff82000) {
                std::cout << "Descriptor tables have base addresses common in VMware environments." << std::endl;
                virtualization_detected = true;
            }

        } else {
            // If an exception occurred, execution jumps here
            std::cout << "SGDT or SIDT caused a segmentation fault. Possible virtualization detected." << std::endl;
            virtualization_detected = true;
        }

        // Restore default signal handler
        std::signal(SIGSEGV, SIG_DFL);

    #elif defined(__aarch64__) 
        cout<<"ARM system detected..."<<endl;
    #endif

    }

    return virtualization_detected;
}


/**
 * x86 Version --
 * Reads the Time Stamp Counter (TSC) before the code block.
 * Ensures serialization to get an accurate timestamp.
 */
uint64_t rdtsc_start() {
#ifdef _MSC_VER
    return __rdtsc();
#elif defined(__x86_64__)
    unsigned int lo, hi;
    // Serialize to ensure all previous instructions have completed
    __asm__ __volatile__(
        "cpuid\n\t"        // Serialize the instruction stream
        "rdtsc\n\t"        // Read the TSC into EDX:EAX
        : "=a"(lo), "=d"(hi) // Output operands
        :
        : "%rbx", "%rcx");   // Clobbered registers
    return ((uint64_t)hi << 32) | lo; // Combine high and low bits
#elif defined(__arm__) || defined(_M_ARM) ||  defined(__aarch64__) || defined(_M_ARM64)
    uint64_t start;
    asm volatile("mrs %0, cntvct_el0" : "=r" (start));
    return start;
#endif
}

/**
 * x86 version --
 * Reads the Time Stamp Counter (TSC) after the code block.
 * Uses RDTSCP which waits for all previous instructions to complete.
 */
uint64_t rdtsc_end() {
#ifdef _MSC_VER
    return __rdtscp();
#elif defined(__x86_64__)
    unsigned int lo, hi;
    // RDTSCP ensures previous instructions have completed, and prevents reordering
    __asm__ __volatile__(
        "rdtscp\n\t"       // Read the TSC and processor ID
        "mov %%edx, %0\n\t"// Move high bits to output variable
        "mov %%eax, %1\n\t"// Move low bits to output variable
        "cpuid\n\t"        // Serialize again
        : "=r"(hi), "=r"(lo) // Output operands
        :
        : "%rax", "%rbx", "%rcx", "%rdx"); // Clobbered registers
    return ((uint64_t)hi << 32) | lo; // Combine high and low bits
#elif defined(__arm__) || defined(_M_ARM) ||  defined(__aarch64__) || defined(_M_ARM64)
    uint64_t end;
    asm volatile("mrs %0, cntvct_el0" : "=r" (end));
    return end;
#endif
}


#if defined(__arm__) || defined(_M_ARM) ||  defined(__aarch64__) || defined(_M_ARM64)
//Function to get frequency of ARM timer (hz)
uint64_t get_arm_frequency(){
    uint64_t frequency;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (frequency));
    return frequency;
}
#endif

bool checkTiming() {
    std::cout << "===== Measuring Timing Discrepancies =====" << std::endl;
    bool detected = false;
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    std::regex pattern("cpu MHz\\s+: ([0-9]+\\.[0-9]+)");
    std::smatch match;
    
if (OS == OS_LINUX) {
    #if defined(__x86_64__) //for x86 intel processors...
        unsigned int cpu_mhz = 0;
        if (cpuinfo.is_open()) 
        {
            while (std::getline(cpuinfo, line)) {
                if (std::regex_search(line, match, pattern)) 
                {
                    double mhz = std::stod(match[1]);
                    cpu_mhz = static_cast<unsigned int>(mhz);
                }
            }
        }
        std::cout << "Detected CPU frequency: " << cpu_mhz << " MHz" << std::endl;

        const int iterations = 1000000;
        uint64_t start, end;
        uint64_t total_cycles = 0;
        cout<<"Iterating nop instructions...."<<endl;
        for (int i = 0; i < iterations; ++i) {
            start = rdtsc_start();

            // Code block to measure
            asm volatile("nop");

            end = rdtsc_end();
            total_cycles += (end - start);
        }

        double average_cycles = total_cycles / static_cast<double>(iterations);
        double average_time_ns = (average_cycles / (cpu_mhz * 1e6)) * 1e9; // Convert to nanoseconds

        std::cout << "Done. \nAverage cycles per operation: " << average_cycles << std::endl;
        std::cout << "Average time per operation: " << average_time_ns << " ns" << std::endl;

        /**
            We know that on an natively running i7-13800H, 
            the above iterations average around 20-50 ns. 
            For the time being, we choose ~50 ns to be the
            threshold for virtualization. More data needed.
        
        */
        // Threshold in nanoseconds (Can probably be improved)
        double threshold_ns = 60.0;
        

        if (average_time_ns > threshold_ns) {
            std::cout << "Timing discrepancies detected. Possible virtualization environment." << std::endl;
            detected = true;
        } 
        else {
            std::cout << "No significant timing discrepancies detected." << std::endl;
        }

    #elif defined(__aarch64__) || defined(_M_ARM64) //for arm processors
        std::cout<<"ARM system detected..."<<endl;
        uint64_t start,end,total_cycles = 0;
        uint64_t frequency = get_arm_frequency();
        std::cout<<"ARM Frequency: " << frequency <<" Hz" << endl;


        const int iterations = 1000000;
        std::cout<<"Iterating nop instructions...."<<endl;

        for(int i = 0; i < iterations; ++i)
        {
            start = rdtsc_start();

            asm volatile("nop");

            end = rdtsc_end();
            total_cycles+= (end - start);
        }
        unsigned int frequency_mhz = frequency / 1e6;


        //average cycles per NOP instruction
        double avg_cycles = total_cycles / static_cast<double>(iterations);
        //average time in ns
        double avg_time = (avg_cycles / frequency_mhz) * 1e3;

        //make sure these calculations are correct lol
        std::cout<<"Done. \nAverage cycles per operation: "<<avg_cycles<<std::endl;
        std::cout<<"Average time per operation: "<< avg_time<<std::endl;
    #endif

    }

    if(OS == OS_WINDOWS)
    {


    }

    return detected;
}


/**
    Test to check PCI vendor and device ID's for virtual Devices

    Seems like we will have two options: try to spoof hardward (hard),
    or just adjust vm settings to allow for a hardware passthrough.
 */
bool checkPCI(){
    cout<<"===== Checking for virtualized PCI devices =====" << endl;
    bool detected = false;
    unsigned int virtualized_devices= 0;
    if (OS == OS_LINUX)
    {
         const std::string pci_path = "/sys/bus/pci/devices";

    if (!std::filesystem::exists(pci_path)) {
        std::cerr << "PCI devices path does not exist." << std::endl;
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(pci_path)) {
        std::string device_path = entry.path();
        std::string vendor_id, device_id;

        try {
            // Read vendor ID
            std::ifstream vendor_file(device_path + "/vendor");
            if (vendor_file) {
                std::getline(vendor_file, vendor_id);
                vendor_file.close();
            } else {
                std::cerr << "Failed to read vendor ID for device: " << device_path << std::endl;
                continue;
            }

            // Read device ID
            std::ifstream device_file(device_path + "/device");
            if (device_file) {
                std::getline(device_file, device_id);
                device_file.close();
            } else {
                std::cerr << "Failed to read device ID for device: " << device_path << std::endl;
                continue;
            }

            // Combine vendor and device IDs in "vendor:device" format
            std::string vendor_device = vendor_id + ":" + device_id;
            // Check if the combined vendor:device ID is in the list of known virtual device IDs
            auto it = std::find(virtual_device_ids.begin(), virtual_device_ids.end(), vendor_device);
            if (it != virtual_device_ids.end()) 
            {
                virtualized_devices++;
                detected = true;
            } 

        } 
        catch (const std::exception& e) {
            std::cerr << "Error processing device " << entry.path().filename() << ": " << e.what() << std::endl;
        }
    } 
    std::cout << "Virtualization artifacts detected for: " 
    << virtualized_devices << " PCI devices. " << std::endl;
    
    
    }
    if (OS == OS_WINDOWS)
    {


    }
    if (OS == OS_MAC)
    {


    }
    return detected;
}

/**
    Test to check our MAC ADDRESS for common VM address prefixes.
 */
bool checkMAC(){
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
bool checkDMI() {
    std::cout << "===== Checking DMI Fields =====" << std::endl;

    // Check for Linux OS
    if(OS == OS_LINUX){
    // Expanded DMI paths, including additional paths for virtualization artifacts
    const std::vector<std::string> dmi_paths = {
        "/sys/class/dmi/id/sys_vendor",
        "/sys/class/dmi/id/product_name",
        "/sys/class/dmi/id/product_version",
        "/sys/class/dmi/id/board_vendor",
        "/sys/class/dmi/id/bios_vendor",
        "/sys/class/dmi/id/product_family",
        "/sys/class/dmi/id/uevent",      // Added path for uevent
        "/sys/class/dmi/id/modalias"     // Added path for modalias
    };

    // Signatures of VM platforms (expand as needed)
    const std::vector<std::string> vm_signatures = {
        "qemu", "vmware", "virtualbox", "hyper-v", "xen", "kvm", "parallels"
    };

    bool detected = false;

    // Check each DMI field for VM signatures
    for (const auto& path : dmi_paths) {
        std::ifstream file(path);
        std::string value, line;

        if (file.is_open()) {
            // Read entire content for multi-line handling
            while (std::getline(file, line)) {
                value += line + " ";
            }

            // Convert content to lowercase to enable case-insensitive searching
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);

            // Check if any VM signature is a substring within the content
            for (const auto& signature : vm_signatures) {
                if (value.find(signature) != std::string::npos) {
                    detected = true;
                    std::cout << "Signature found: \"" << signature << "\" in DMI field: " << path << std::endl;
                }
            }
        } else {
            std::cout << "Could not open file: " << path << std::endl;
        }
    }

    // Additional check using dmidecode
    std::cout << "===== Checking dmidecode Output =====" << std::endl;
    // Command to execute
    const char* cmd = "dmidecode --type system 2>/dev/null";

    // Open the command for reading
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        std::cerr << "Could not run dmidecode command." << std::endl;
        return detected; // Return current detection status
    }

    std::array<char, 256> buffer;
    std::string dmidecode_output;

    // Read the output of dmidecode
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        dmidecode_output += buffer.data();
    }

    // Convert dmidecode output to lowercase
    std::transform(dmidecode_output.begin(), dmidecode_output.end(), dmidecode_output.begin(), ::tolower);

    // Check for VM signatures in dmidecode output
    for (const auto& signature : vm_signatures) {
        if (dmidecode_output.find(signature) != std::string::npos) {
            detected = true;
            std::cout << "Signature found: \"" << signature << "\" in dmidecode output." << std::endl;
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
bool checkVendorID() {
    std::cout << "===== Checking Hypervisor Vendor ID =====" << std::endl;

#if defined(__x86_64__)
    if (OS == OS_LINUX) 
    {
        unsigned int eax, ebx, ecx, edx;
        char hyper_vendor[13] = {0};

        // Step 1: Check if hypervisor is present by examining the hypervisor present bit
        eax = 1; // CPUID leaf 0x1
        __asm__ __volatile__(
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)  // Outputs
            : "0"(eax)                                     // Inputs
        );

        if (!(ecx & (1 << 31))) {
            printf("No hypervisor detected (hypervisor bit not set).\n");
            return false;
        }

        // Step 2: Query hypervisor vendor ID
        eax = 0x40000000; // Hypervisor CPUID leaf
        __asm__ __volatile__(
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)  // Outputs
            : "0"(eax)                                     // Inputs
        );

        memcpy(hyper_vendor + 0, &ebx, 4);
        memcpy(hyper_vendor + 4, &ecx, 4);
        memcpy(hyper_vendor + 8, &edx, 4);
        hyper_vendor[12] = '\0';

        if (strlen(hyper_vendor) > 0) 
        {
            printf("Hypervisor Vendor ID: %s\n", hyper_vendor);
            return true;
        } 
        else 
        {
            printf("No Hypervisor Vendor ID found.\n");
            return false;
        }
    }
    else
    {
        printf("Unsupported OS for hypervisor detection on x86 architecture.\n");
        return false;
    }
#elif defined(__arm__) || defined(_M_ARM) ||  defined(__aarch64__) || defined(_M_ARM64)
    if (OS == OS_LINUX)
    {
        // ARM-specific virtualization detection could be added here
        // For now, we'll indicate that this operation is unsupported
        std::cout << "Hypervisor detection via CPUID is not supported on ARM architectures.\n";
        return false;
    }
    else
    {
        std::cout << "Unsupported OS for hypervisor detection on ARM architecture.\n";
        return false;
    }
#else
    std::cout << "Unsupported architecture for hypervisor detection.\n";
    return false;
#endif
}



/**
    Test to check whether the hypervisor bit is set.
 */
bool checkHypervisorBit() 
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

#elif defined(__arm__) || defined(_M_ARM) ||  defined(__aarch64__) || defined(_M_ARM64)
    if (OS == OS_LINUX)
    {
        cout<<"On arm based processor, implement check hypervisor bit"<<endl;
    }
#else
    std::cout << "Unsupported architecture for hypervisor detection." << std::endl;
    return false;
#endif
    return false;

}

/**
    Test to check if IO Devices have VM signatures attached.
 */
bool checkIODevices() 
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



