#ifndef VM_MITIGATIONS_H
#define VM_MITIGATIONS_H

#include <map>
#include <string>

void applyMitigations(const std::map<std::string, bool> test_results);

#endif // VM_MITIGATIONS_H