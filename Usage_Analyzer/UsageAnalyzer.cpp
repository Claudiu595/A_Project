#include "UsageAnalyzer.h"
#include "ReservedWords.h"
#include <regex>
#include <unordered_map>

void UsageAnalyzer::count_usages(std::vector<Symbol>& symbols, const std::vector<std::filesystem::path>& files, FileCache& file_cache)
{
	static const std::regex identifier_regex(R"([A-Za-z_][A-Za-z0-9_]*)");

	static const std::regex declaration_regex(R"(^\s*([A-Za-z_][A-Za-z0-9_\s\*&:<>]*)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*;\s*$)");

	std::unordered_map<std::string, size_t> occurrences;
	std::unordered_map<std::string, size_t> declaration_counts;

	for (const auto& path : files)
	{
		const auto& lines = get_cached_lines(file_cache, path);

		for (const auto& line : lines)
		{
			auto begin = std::sregex_iterator(line.begin(), line.end(), identifier_regex);
			auto end = std::sregex_iterator();

			for (auto it = begin; it != end; ++it)
			{
				occurrences[it->str()]++;
			}

			std::smatch match;
			if (std::regex_search(line, match, declaration_regex))
			{
				std::string prefix = match[1].str();
				std::string name = match[2].str();

				size_t pos = prefix.find_last_of(" \t");
				std::string last_prefix_token = (pos == std::string::npos) ? prefix : prefix.substr(pos + 1);

				if (!is_reserved_word(name) && !is_reserved_word(last_prefix_token))
				{
					declaration_counts[name]++;
				}
			}
		}
	}

	for (auto& symbol : symbols)
	{
		size_t count = occurrences[symbol.name];

		if (symbol.is_macro)
		{
			symbol.usage_count = (count > 0) ? (count - 1) : 0;
			continue;
		}

		if (symbol.name == "main")
		{
			symbol.usage_count = 1;
			continue;
		}

		size_t discount = 1 + declaration_counts[symbol.name];
		symbol.usage_count = (count > discount) ? (count - discount) : 0;
	}
}
