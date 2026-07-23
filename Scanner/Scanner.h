#pragma once
#include <vector>
#include <string>
#include <filesystem>

class Scanner
{
public:
	std::vector<std::filesystem::path> scan_folder(const std::string& folder_path);
};
