#pragma once




#include <string>					// string









namespace StringUtils
{
	std::string	toLower(const std::string &s);
	std::string	trim(const std::string &s);
	bool		isAllDigits(const std::string &s, size_t start = 0, size_t end = std::string::npos);
	bool		hasInvalidChar(const std::string &s, const std::string &invalidChars, size_t from = 0);
}
