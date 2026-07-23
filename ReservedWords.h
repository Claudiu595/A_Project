#pragma once
#include <string>
#include <unordered_set>

inline bool is_reserved_word(const std::string& word)
{
	static const std::unordered_set<std::string> reserved = {
		"if", "else", "while", "for", "switch", "return", "do", "sizeof",
		"case", "goto", "break", "continue", "typedef", "struct", "union",
		"enum", "namespace", "using", "new", "delete", "throw", "catch",
		"try", "static_assert", "alignof", "decltype"
	};
	return reserved.count(word) != 0;
}
