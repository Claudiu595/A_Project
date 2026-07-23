#pragma once
#include "Symbol.h"
#include <vector>
#include <string>
#include <unordered_map>

class DuplicateChecker {
public:
    std::unordered_map<std::string, std::string> check_duplicates(
        const std::vector<Symbol>& symbols,
        std::vector<std::string>& advices_out);
};
