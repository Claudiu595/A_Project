#include "Extractor.h"
#include "ReservedWords.h"
#include <regex>

namespace
{
	void count_braces_in_line(const std::string& line, int& braces, bool& in_block_comment, bool& started)
	{
		bool in_string = false;
		bool in_char = false;

		for (size_t idx = 0; idx < line.size(); ++idx)
		{
			char c = line[idx];
			char next = (idx + 1 < line.size()) ? line[idx + 1] : '\0';

			if (in_block_comment)
			{
				if (c == '*' && next == '/') 
				{ 
					in_block_comment = false; 
					++idx; 
				}
				continue;
			}
			if (in_string)
			{
				if (c == '\\') 
				{ 
					++idx; 
					continue; 
				}
				if (c == '"') in_string = false;
				continue;
			}
			if (in_char)
			{
				if (c == '\\') 
				{ 
					++idx; 
					continue; 
				}
				if (c == '\'') in_char = false;
				continue;
			}

			if (c == '/' && next == '/') 
				break;
			if (c == '/' && next == '*')
			{ 
				in_block_comment = true; 
				++idx; 
				continue;
			}
			if (c == '"')
			{ 
				in_string = true;
				continue;
			}
			if (c == '\'') 
			{ 
				in_char = true; 
				continue; 
			}

			if (c == '{') { 
				++braces; 
				started = true; 
			}
			else if (c == '}') --braces;
		}
	}
}

std::vector<Symbol> Extractor::extract_symbols(const std::vector<std::filesystem::path>& files, FileCache& file_cache)
{
	std::vector<Symbol> symbols;

	static const std::regex macro_regex(R"(^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)\s*(.*)?$)");
	static const std::regex function_regex(R"(^\s*[A-Za-z_][A-Za-z0-9_\s\*&:<>]*\s+([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*(\{.*)?$)");

	for (const auto& path : files)
	{
		const auto& lines = get_cached_lines(file_cache, path);

		for (size_t i = 0; i < lines.size(); ++i)
		{
			std::smatch match;

			if (std::regex_search(lines[i], match, macro_regex))//is macro
			{
				Symbol symbol;
				symbol.name = match[1];
				symbol.value = match[2];
				symbol.is_macro = true;
				symbol.filepath = path;
				symbol.line_start = static_cast<int>(i + 1);
				symbol.line_end = static_cast<int>(i + 1);
				symbols.push_back(symbol);
				continue;
			}

			if (std::regex_search(lines[i], match, function_regex))//is a function
			{
				std::string name = match[1].str();
				if (is_reserved_word(name)) continue;

				int braces = 0;
				bool in_block_comment = false;
				bool started = false;
				int line_end = -1;

				for (size_t j = i; j < lines.size(); ++j)
				{
					count_braces_in_line(lines[j], braces, in_block_comment, started);

					if (started && braces == 0)
					{
						line_end = static_cast<int>(j + 1);
						break;
					}

					if (!started && (j - i) >= 5) break;
				}

				if (line_end == -1) continue;

				Symbol symbol;
				symbol.name = name;
				symbol.value = lines[i];
				symbol.is_macro = false;
				symbol.filepath = path;
				symbol.line_start = static_cast<int>(i + 1);
				symbol.line_end = line_end;
				symbols.push_back(symbol);
			}
		}
	}
	return symbols;
}
