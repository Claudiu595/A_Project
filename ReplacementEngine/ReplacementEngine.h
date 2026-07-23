#pragma once
#include "Symbol.h"
#include "FileCache.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>

class ReplacementEngine
{
public:
	static void apply_replacements(
		const std::unordered_map<std::string, std::string>& replacements,
		const std::vector<Symbol>& symbols,
		const std::vector<std::filesystem::path>& files,
		FileCache& file_cache);
};
