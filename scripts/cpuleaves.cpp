/**
    CPUID output into readable format. Checks CPU leaves. 
 */


#include <iostream>
#include <vector>
#include <array>
#include <bitset>
#include <string>
#include <cstring>
// Structure to hold CPUID information
struct CPUIDInfo {
    unsigned int eax, ebx, ecx, edx;
};

// Function to call the CPUID instruction
CPUIDInfo cpuid(unsigned int leaf, unsigned int subleaf = 0) {
    CPUIDInfo info = {0, 0, 0, 0};
    #if defined(_MSC_VER) // For MSVC
        int cpuInfo[4];
        __cpuidex(cpuInfo, leaf, subleaf);
        info.eax = cpuInfo[0];
        info.ebx = cpuInfo[1];
        info.ecx = cpuInfo[2];
        info.edx = cpuInfo[3];
    #elif defined(__GNUC__) || defined(__clang__) // For GCC/Clang
        __asm__ volatile("cpuid"
                         : "=a"(info.eax), "=b"(info.ebx), "=c"(info.ecx), "=d"(info.edx)
                         : "a"(leaf), "c"(subleaf));
    #endif
    return info;
}

// Retrieve CPU vendor
std::string get_vendor() {
    CPUIDInfo info = cpuid(0);
    char vendor[13];
    memcpy(vendor, &info.ebx, 4);
    memcpy(vendor + 4, &info.edx, 4);
    memcpy(vendor + 8, &info.ecx, 4);
    vendor[12] = '\0';
    return std::string(vendor);
}

// Retrieve processor brand string
std::string get_brand_string() {
    char brand[49] = {};
    for (unsigned int i = 0; i < 3; ++i) {
        CPUIDInfo info = cpuid(0x80000002 + i);
        memcpy(brand + i * 16, &info, 16);
    }
    return std::string(brand);
}

// Decode feature flags from Leaf 0x1 and Leaf 0x7
void print_feature_flags() {
    // Leaf 0x1 for basic feature flags
    CPUIDInfo info1 = cpuid(0x1);

    // Leaf 0x7 for extended feature flags
    CPUIDInfo info7 = cpuid(0x7, 0x0);

    std::cout << "Feature Flags:" << std::endl;
    std::cout << "  SSE3 Support: " << ((info1.ecx & (1 << 0)) ? "Yes" : "No") << std::endl;
    std::cout << "  AVX Support: " << ((info1.ecx & (1 << 28)) ? "Yes" : "No") << std::endl;
    std::cout << "  FMA Support: " << ((info1.ecx & (1 << 12)) ? "Yes" : "No") << std::endl;
    std::cout << "  SSE4.1 Support: " << ((info1.ecx & (1 << 19)) ? "Yes" : "No") << std::endl;
    std::cout << "  SSE4.2 Support: " << ((info1.ecx & (1 << 20)) ? "Yes" : "No") << std::endl;

    // Leaf 0x7 flags (e.g., AVX2, BMI1, BMI2)
    std::cout << "  AVX2 Support: " << ((info7.ebx & (1 << 5)) ? "Yes" : "No") << std::endl;
    std::cout << "  BMI1 Support: " << ((info7.ebx & (1 << 3)) ? "Yes" : "No") << std::endl;
    std::cout << "  BMI2 Support: " << ((info7.ebx & (1 << 8)) ? "Yes" : "No") << std::endl;
    std::cout << "  SHA Support: " << ((info7.ebx & (1 << 29)) ? "Yes" : "No") << std::endl;
}

// Main function to display CPUID information
int main() {
    std::cout << "CPU Vendor: " << get_vendor() << std::endl;
    std::cout << "Processor Brand: " << get_brand_string() << std::endl;

    print_feature_flags();

    return 0;
}
