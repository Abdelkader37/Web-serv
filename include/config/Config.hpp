#pragma once

#include "config/VirtualHost.hpp"	// VirtualHost

#include <string>					// string
#include <vector>					// vector
#include <fstream>					// ifstream

class Config
{

public:

struct Token
{

enum TokenType
{

	DirectiveWord,
	DirectiveDelimiter,
	BlockOpen,
	BlockClose

};
	Token() : type(DirectiveWord) {};
	Token(const std::string &inputContent, TokenType inputType) : content(inputContent), type(inputType) { };

	std::string	content;
	TokenType	type;

};

const std::vector<VirtualHost>	&parse(const std::string &filepath);

private:

	void	parseServerBlock(size_t &cursor);

	void	tokenize(std::ifstream &configIfs);

	std::vector<Token>			tokens_;
	std::vector<VirtualHost>	virtualHosts_;

};
