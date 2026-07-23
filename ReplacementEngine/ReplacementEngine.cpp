#include "ReplacementEngine.h"
#include <fstream>
#include <regex>
#include <iostream>

void ReplacementEngine::apply_replacements(
	const std::unordered_map<std::string, std::string>& replacements,
	const std::vector<Symbol>& symbols,
	const std::vector<std::filesystem::path>& files,
	FileCache& file_cache)
{
	if (replacements.empty()) return;

	std::unordered_map<std::filesystem::path, std::unordered_map<int, std::string>> lines_to_neutralize;
	for (const auto& s : symbols)
	{
		if (s.is_macro && replacements.count(s.name))
			lines_to_neutralize[s.filepath][s.line_start] = s.name;
	}

	for (const auto& path : files)
	{
		std::vector<std::string> lines = get_cached_lines(file_cache, path);
		auto file_it = lines_to_neutralize.find(path);

		for (size_t i = 0; i < lines.size(); ++i)
		{
			int currentLine = static_cast<int>(i + 1);

			if (file_it != lines_to_neutralize.end())
			{
				auto line_it = file_it->second.find(currentLine);
				if (line_it != file_it->second.end())
				{
					const std::string& old_name = line_it->second;
					const std::string& canonical = replacements.at(old_name);
					lines[i] = "// [MERGED] '" + old_name + "' definition removed (duplicate of '" + canonical + "')";
					continue;
				}
			}

			for (const auto& replacement : replacements)
			{
				std::regex pattern("\\b" + replacement.first + "\\b");
				lines[i] = std::regex_replace(lines[i], pattern, replacement.second);
			}
		}

		std::ofstream output(path);
		if (!output) continue;

		for (const auto& currentLine : lines)
		{
			output << currentLine << '\n';
		}
		output.close();

		file_cache[path] = std::move(lines);
	}

	std::cout << "Macro fusions applied successfully.\n";
}
