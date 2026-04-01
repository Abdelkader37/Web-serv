#pragma once




#include <string>					// string









namespace StringUtils
{
	std::string	toLower(const std::string &s);
	std::string	trim(const std::string &s);
	bool		isAllDigits(const std::string &s, size_t start, size_t end);
	bool		hasInvalidChar(const std::string &s, const std::string &invalidChars, size_t from = 0);
}
