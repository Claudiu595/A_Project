#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

using FileCache = std::unordered_map<std::filesystem::path, std::vector<std::string>>;

inline const std::vector<std::string>& get_cached_lines(FileCache& cache, const std::filesystem::path& path)
{
	auto it = cache.find(path);
	if (it != cache.end()) return it->second;

	std::vector<std::string> lines;
	std::ifstream file(path);
	if (file)
	{
		std::string line;
		while (std::getline(file, line)) lines.push_back(line);
	}
	auto [inserted, _] = cache.emplace(path, std::move(lines));
	return inserted->second;
}
