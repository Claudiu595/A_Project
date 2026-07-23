#include "Refactorer.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <unordered_map>

void Refactorer::apply_refactoring(
	int option,
	const std::vector<Symbol>& symbols,
	const std::vector<std::filesystem::path>& files,
	FileCache& file_cache)
{
	if (option == 1) return;

	std::unordered_map<std::filesystem::path, std::vector<const Symbol*>> unused_by_file;
	for (const auto& symbol : symbols)
	{
		if (symbol.usage_count == 0) unused_by_file[symbol.filepath].push_back(&symbol);
	}

	for (auto& [path, syms] : unused_by_file)
	{
		std::sort(syms.begin(), syms.end(), [](const Symbol* a, const Symbol* b) { return a->line_start < b->line_start; });
	}

	for (const auto& path : files)
	{
		const auto& lines = get_cached_lines(file_cache, path);

		std::ofstream output(path);
		if (!output) continue;

		std::vector<const Symbol*>* file_symbols = nullptr;
		if (auto it = unused_by_file.find(path); it != unused_by_file.end())
			file_symbols = &it->second;

		size_t sym_idx = 0;
		std::vector<const Symbol*> active;

		for (size_t i = 0; i < lines.size(); ++i)
		{
			int currentLine = static_cast<int>(i + 1);

			if (file_symbols)
			{
				while (sym_idx < file_symbols->size() && (*file_symbols)[sym_idx]->line_start <= currentLine)
				{
					active.push_back((*file_symbols)[sym_idx]);
					++sym_idx;
				}

				active.erase(std::remove_if(active.begin(), active.end(),
					[currentLine](const Symbol* s) { return s->line_end < currentLine; }), active.end());
			}

			if (option == 2)
			{
				for (const Symbol* s : active)
				{
					if (s->line_start == currentLine)
					{
						output << "/* UNUSED SYMBOL: " << s->name << " - Ready for removal */\n";
					}
				}
			}

			if (option == 3 && !active.empty()) continue;

			output << lines[i] << '\n';
		}
		output.close();
	}
}
