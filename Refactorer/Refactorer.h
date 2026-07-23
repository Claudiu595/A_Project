#pragma once
#include "Symbol.h"
#include "FileCache.h"
#include <vector>
#include <filesystem>

class Refactorer {
public:
    void apply_refactoring(int option, const std::vector<Symbol>& symbols, const std::vector<std::filesystem::path>& files, FileCache& file_cache);
};
