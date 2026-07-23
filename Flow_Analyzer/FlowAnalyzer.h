#pragma once
#include "Symbol.h"
#include "FileCache.h"
#include <vector>

class FlowAnalyzer
{
public:
	void analyze_reachability(std::vector<Symbol>& symbols, FileCache& file_cache);
};
