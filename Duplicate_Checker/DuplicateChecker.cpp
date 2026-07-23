#include "DuplicateChecker.h"
#include <unordered_set>

std::unordered_map<std::string, std::string> DuplicateChecker::check_duplicates(
	const std::vector<Symbol>& symbols,
	std::vector<std::string>& advices_out)
{
	std::unordered_map<std::string, std::string> replacements;
	if (symbols.empty()) return replacements;

	std::unordered_map<std::string, std::vector<const Symbol*>> by_value;
	for (const auto& s : symbols)
	{
		if (s.is_macro && !s.value.empty())
			by_value[s.value].push_back(&s);
	}

	for (const auto& [value, group] : by_value)
	{
		if (group.size() < 2) continue;

		const std::string& canonical_name = group[0]->name;
		std::unordered_set<std::string> seen_names = { canonical_name };

		for (size_t k = 1; k < group.size(); ++k)
		{
			const std::string& other_name = group[k]->name;
			if (seen_names.count(other_name)) continue;
			seen_names.insert(other_name);

			replacements[other_name] = canonical_name;

			std::string advice;
			advice += "[POTENTIAL FUSION]\n";
			advice += "Macro: " + canonical_name + "\n";
			advice += "Macro: " + other_name + "\n";
			advice += "Same Value: " + value + "\n\n";
			advice += "WARNING: Can be merged. \n" + other_name + " can be replaced with " + canonical_name + "\n";
			advices_out.push_back(advice);
		}
	}

	std::unordered_map<std::string, std::vector<const Symbol*>> by_name;
	for (const auto& s : symbols)
	{
		if (s.is_macro) by_name[s.name].push_back(&s);
	}

	for (const auto& [name, group] : by_name)
	{
		if (group.size() < 2) continue;

		for (size_t a = 0; a < group.size(); ++a)
		{
			for (size_t b = a + 1; b < group.size(); ++b)
			{
				const Symbol* s1 = group[a];
				const Symbol* s2 = group[b];

				if (s1->filepath == s2->filepath) continue;
				if (s1->value == s2->value) continue;

				std::string advice;
				advice += "[CONFLICT]\n";
				advice += name + " appears in multiple files with different values.\n";
				advice += " - " + s1->filepath.filename().string() + ":" + std::to_string(s1->line_start) + " = " + s1->value + "\n";
				advice += " - " + s2->filepath.filename().string() + ":" + std::to_string(s2->line_start) + " = " + s2->value + "\n";
				advices_out.push_back(advice);
			}
		}
	}

	return replacements;
}
