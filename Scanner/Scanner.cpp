#include "Scanner.h"
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

std::vector<fs::path> Scanner::scan_folder(const std::string& folder_path)
{
	std::vector<fs::path> files;

	if (!fs::exists(folder_path) || !fs::is_directory(folder_path))
	{
		std::cout << "Error: Invalid folder path!\n";
		return files;
	}

	std::error_code ec;
	fs::recursive_directory_iterator it(folder_path, fs::directory_options::skip_permission_denied, ec);
	fs::recursive_directory_iterator end;

	if (ec)
	{
		std::cout << "Error: Cannot scan folder (" << ec.message() << ")\n";
		return files;
	}

	while (it != end)
	{
		std::error_code entry_ec;
		bool is_file = it->is_regular_file(entry_ec);

		if (!entry_ec && is_file)
		{
			auto ext = it->path().extension();
			if (ext == ".c" || ext == ".h" || ext == ".cpp" || ext == ".hpp")
			{
				files.push_back(it->path());
			}
		}

		it.increment(ec);
		if (ec)
		{
			std::cout << "Warning: Stopping traversal early (" << ec.message() << ")\n";
			break;
		}
	}

	std::cout << files.size() << " source files found.\n";
	return files;
}
