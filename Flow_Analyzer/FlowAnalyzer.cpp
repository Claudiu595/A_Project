#include "FlowAnalyzer.h"
#include <regex>
#include <unordered_map>
#include <unordered_set>

namespace
{
	void collect_known_names(const std::string& line, const std::unordered_set<std::string>& known, std::unordered_set<std::string>& out)
	{
		static const std::regex identifier_regex(R"([A-Za-z_][A-Za-z0-9_]*)");
		for (auto it = std::sregex_iterator(line.begin(), line.end(), identifier_regex); it != std::sregex_iterator(); ++it)
		{
			if (known.count(it->str())) out.insert(it->str());
		}
	}

	bool is_pure_prototype_line(const std::string& line)
	{
		static const std::regex declaration_regex(R"(^\s*[A-Za-z_][A-Za-z0-9_\s\*&:<>]*\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^;{}]*\)\s*;\s*$)");
		return std::regex_search(line, declaration_regex);
	}
}

void FlowAnalyzer::analyze_reachability(std::vector<Symbol>& symbols, FileCache& file_cache)
{
	bool has_main = false;
	for (const auto& s : symbols)
	{
		if (!s.is_macro && s.name == "main") { has_main = true; break; }
	}
	if (!has_main) return;

	std::unordered_set<std::string> function_names;
	for (const auto& s : symbols)
		if (!s.is_macro) function_names.insert(s.name);

	std::unordered_map<std::filesystem::path, std::vector<int>> line_owner;
	for (size_t idx = 0; idx < symbols.size(); ++idx)
	{
		const Symbol& s = symbols[idx];
		if (s.is_macro) continue;

		const auto& lines = get_cached_lines(file_cache, s.filepath);
		auto& owner = line_owner[s.filepath];
		if (owner.empty()) owner.assign(lines.size(), -1);

		for (int ln = s.line_start; ln <= s.line_end && ln >= 1 && static_cast<size_t>(ln) <= owner.size(); ++ln)
			owner[ln - 1] = static_cast<int>(idx);
	}

	std::unordered_map<std::string, std::unordered_set<std::string>> call_graph;
	for (const auto& s : symbols)
	{
		if (s.is_macro) continue;

		auto& callees = call_graph[s.name];
		const auto& lines = get_cached_lines(file_cache, s.filepath);
		for (int ln = s.line_start; ln <= s.line_end && ln >= 1 && static_cast<size_t>(ln) <= lines.size(); ++ln)
			collect_known_names(lines[ln - 1], function_names, callees);
	}
	for (auto& entry : call_graph)
		entry.second.erase(entry.first);

	std::unordered_set<std::string> reachable;
	std::vector<std::string> worklist;
	auto seed = [&](const std::string& name)
	{
		if (function_names.count(name) && reachable.insert(name).second)
			worklist.push_back(name);
	};
	seed("main");

	static const std::vector<int> no_functions_in_file;
	for (const auto& cache_entry : file_cache)
	{
		const auto& path = cache_entry.first;
		const auto& lines = cache_entry.second;
		auto owner_it = line_owner.find(path);
		const auto& owner = (owner_it != line_owner.end()) ? owner_it->second : no_functions_in_file;

		for (size_t i = 0; i < lines.size(); ++i)
		{
			if (i < owner.size() && owner[i] != -1) continue;
			if (is_pure_prototype_line(lines[i])) continue;
			std::unordered_set<std::string> found;
			collect_known_names(lines[i], function_names, found);
			for (const auto& name : found) seed(name);
		}
	}

	while (!worklist.empty())
	{
		std::string current = worklist.back();
		worklist.pop_back();

		auto it = call_graph.find(current);
		if (it == call_graph.end()) continue;
		for (const auto& callee : it->second) seed(callee);
	}

	for (auto& s : symbols)
	{
		if (s.is_macro) continue;
		if (!reachable.count(s.name)) s.usage_count = 0;
	}

	std::unordered_map<std::string, size_t> live_occurrences;
	static const std::regex identifier_regex(R"([A-Za-z_][A-Za-z0-9_]*)");

	for (const auto& cache_entry : file_cache)
	{
		const auto& path = cache_entry.first;
		const auto& lines = cache_entry.second;
		auto owner_it = line_owner.find(path);
		const auto& owner = (owner_it != line_owner.end()) ? owner_it->second : no_functions_in_file;

		for (size_t i = 0; i < lines.size(); ++i)
		{
			bool is_global_scope = !(i < owner.size() && owner[i] != -1);
			bool line_is_live = is_global_scope || reachable.count(symbols[owner[i]].name) != 0;
			if (!line_is_live) continue;

			for (auto it = std::sregex_iterator(lines[i].begin(), lines[i].end(), identifier_regex);
				 it != std::sregex_iterator(); ++it)
				live_occurrences[it->str()]++;
		}
	}

	for (auto& s : symbols)
	{
		if (!s.is_macro) continue;
		size_t count = live_occurrences[s.name];
		s.usage_count = (count > 0) ? (count - 1) : 0;
	}
}
