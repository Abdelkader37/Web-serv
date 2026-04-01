#include "utils/StringUtils.hpp"	// toLower, trim, isAllDigits, hasInvalidChar

std::string StringUtils::toLower(const std::string &s)
{
	std::string result = s;
	for (size_t i = 0; i < result.size(); ++i)
		result.at(i) = std::tolower((unsigned char)result.at(i));
	return result;
}

std::string StringUtils::trim(const std::string &s)
{
	size_t start = 0;
	while (start < s.size() && std::isspace((unsigned char)s.at(start)))
		++start;
	size_t end = s.size();
	while (end > start && std::isspace((unsigned char)s.at(end - 1)))
		--end;
	return s.substr(start, end - start);
}

bool StringUtils::isAllDigits(const std::string &s, size_t start, size_t end)
{
	if (start >= end)
		return false;
	for (size_t i = start; i < end; i++)
		if (!std::isdigit((unsigned char)s.at(i)))
			return false;
	return true;
}

bool StringUtils::hasInvalidChar(const std::string &s, const std::string &invalid, size_t from)
{
	return s.find_first_of(invalid, from) != std::string::npos;
}
