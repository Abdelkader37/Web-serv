#include "utils/StringUtils.hpp"

std::string StringUtils::toLower(const std::string &s)
{
	std::string result = s;
	for (size_t i = 0; i < result.size(); ++i)
		result[i] = std::tolower((unsigned char)result[i]);
	return result;
}

std::string StringUtils::trim(const std::string &s)
{
	size_t start = 0;
	while (start < s.size() && std::isspace((unsigned char)s[start]))
		++start;
	size_t end = s.size();
	while (end > start && std::isspace((unsigned char)s[end - 1]))
		--end;
	return s.substr(start, end - start);
}
