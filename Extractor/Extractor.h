#pragma once
#include "Symbol.h"
#include "FileCache.h"
#include <vector>

class Extractor
{
public:
	std::vector<Symbol> extract_symbols(const std::vector<std::filesystem::path>& files, FileCache& file_cache);
};
