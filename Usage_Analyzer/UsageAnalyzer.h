#pragma once
#include "Symbol.h"
#include "FileCache.h"
#include <vector>

class UsageAnalyzer
{
public:
	void count_usages(std::vector<Symbol>& symbols, const std::vector<std::filesystem::path>& files, FileCache& file_cache);
};
