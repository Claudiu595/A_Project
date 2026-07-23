#include "Scanner/Scanner.h"
#include "Extractor/Extractor.h"
#include "Usage_Analyzer/UsageAnalyzer.h"
#include "Duplicate_Checker/DuplicateChecker.h"
#include "Refactorer/Refactorer.h"
#include "ReplacementEngine/ReplacementEngine.h"
#include "Flow_Analyzer/FlowAnalyzer.h"
#include "FileCache.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

int main()
{
	std::string path;

	std::cout << "Enter folder path: ";
	std::getline(std::cin, path);

	Scanner scanner;
	Extractor extractor;
	UsageAnalyzer analyzer;
	FlowAnalyzer flow_analyzer;
	DuplicateChecker checker;
	Refactorer refactorer;

	std::cout << "\nScanning folder...\n";
	auto files = scanner.scan_folder(path);

	if (files.empty())
	{
		std::cout << "No source files found. Exiting.\n";
		return 0;
	}

	FileCache file_cache;

	std::cout << "Extracting functions and macros...\n";
	auto symbols = extractor.extract_symbols(files, file_cache);

	std::cout << "Calculating usage metrics...\n";
	analyzer.count_usages(symbols, files, file_cache);

	std::cout << "Analyzing execution flow from main()...\n";
	flow_analyzer.analyze_reachability(symbols, file_cache);

	std::cout << "Checking for duplicates and conflicts...\n";
	std::vector<std::string> advices;
	auto replacements = checker.check_duplicates(symbols, advices);

	if (advices.empty())
		std::cout << "No duplicate macros or conflicts found.\n";
	else
		std::cout << advices.size() << " optimization suggestion(s) found.\n";

	size_t conflict_count = 0;
	for (const auto& advice : advices)
		if (advice.rfind("[CONFLICT]", 0) == 0) ++conflict_count;

	int dead_count = 0;
	std::vector<std::string> unused_lines;

	for (const auto& s : symbols)
	{
		if (s.is_macro && s.name.find("_H_INCLUDED") != std::string::npos)
		{
			continue;
		}

		if (s.usage_count == 0)
		{
			std::string line = std::string(" - ") + (s.is_macro ? "[Macro] " : "[Function] ") + s.name
				+ " (" + s.filepath.filename().string() + ":" + std::to_string(s.line_start) + ")";
			unused_lines.push_back(line);
			++dead_count;
		}
	}

	std::cout << "\nUnused Symbols:\n";
	if (unused_lines.empty())
		std::cout << "None found.\n";
	for (const auto& line : unused_lines)
		std::cout << line << "\n";

	std::cout << "\nTotal unused symbols found: " << dead_count << "\n";

	std::filesystem::path report_path = std::filesystem::path(path) / "OptimizationReport.txt";
	std::ofstream report(report_path);
	if (report)
	{
		report << "Unused Symbols \n";
		if (unused_lines.empty())
			report << "None found.\n";
		else
			for (const auto& line : unused_lines) report << line << "\n";

		report << "\n=== Optimization Suggestions ===\n";
		if (advices.empty())
			report << "No duplicate macros or conflicts found.\n";
		else
			for (const auto& advice : advices) report << advice << "\n";

		report.close();
		std::cout << "\nOptimization report saved to: " << report_path.string() << "\n";
	}
	else
	{
		std::cout << "\nWarning: Could not write optimization report to " << report_path.string() << "\n";
	}

	if (dead_count == 0 && replacements.empty() && advices.empty())
	{
		std::cout << "No cleanable code detected. Exiting.\n";
		return 0;
	}

	std::cout << "\nAction Menu:\n";
	std::cout << "1. Do nothing\n";
	std::cout << "2. Comment unused symbols (\"Ready for removal\")\n";
	std::cout << "3. Remove unused macros, comment unused functions automatically\n";
	std::cout << "4. Optimize the project (apply every suggestion from the report)\n";
	std::cout << "Option: ";

	int option;
	std::cin >> option;

	switch (option)
	{
	case 1:
		std::cout << "\nNo changes applied.\n";
		break;
	case 2:
		refactorer.apply_refactoring(2, symbols, files, file_cache);
		std::cout << "\nComments added successfully.\n";
		break;
	case 3:
		refactorer.apply_refactoring(3, symbols, files, file_cache);
		std::cout << "\nUnused macros removed, unused functions commented successfully.\n";
		break;
	case 4:
	{
		bool merged = !replacements.empty();
		bool had_dead_code = (dead_count > 0);

		if (merged)
		{
			ReplacementEngine::apply_replacements(replacements, symbols, files, file_cache);
			std::cout << "Recalculating usage metrics post-fusion...\n";
			analyzer.count_usages(symbols, files, file_cache);
			flow_analyzer.analyze_reachability(symbols, file_cache);
		}

		refactorer.apply_refactoring(3, symbols, files, file_cache);

		std::cout << "\nProject optimized: ";
		if (merged && had_dead_code)
			std::cout << "duplicate macros merged, unused macros removed, and unused functions commented.\n";
		else if (merged)
			std::cout << "duplicate macros merged.\n";
		else if (had_dead_code)
			std::cout << "unused macros removed and unused functions commented.\n";
		else
			std::cout << "no automatic fixes were applicable.\n";

		if (conflict_count > 0)
			std::cout << conflict_count << " conflict(s) require manual review (see report) and were not changed automatically.\n";
		break;
	}
	default:
		std::cout << "\nInvalid selection.\n";
		break;
	}

	return 0;
}
