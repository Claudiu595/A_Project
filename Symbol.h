#pragma once

#include <string>
#include <filesystem>

struct Symbol
{
	std::string name;
	std::string value;
	bool is_macro = false;
	std::filesystem::path filepath;
	int line_start = 0;
	int line_end = 0;
	size_t usage_count = 0;
};
