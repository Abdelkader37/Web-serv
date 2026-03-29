#include "config/Config.hpp"		// Config, parse, tokenize
#include <cstdio>					// EOF
#include <limits>					// numeric_limits

#define DefaultConfig InstallDir "/conf/webserv.conf"

void Config::tokenize(std::ifstream &configIfs)
{
	static const std::string delimiters("{};\n\t ");
	int next;
	while ((next = configIfs.peek()) != EOF)
	{
		switch (next)
		{
		case '{': tokens_.push_back(Token("{", Token::BlockOpen));			configIfs.ignore(); break;
		case '}': tokens_.push_back(Token("}", Token::BlockClose));			configIfs.ignore(); break;
		case ';': tokens_.push_back(Token(";", Token::DirectiveDelimiter));	configIfs.ignore(); break;
		case ' ': case '\t': case '\n': case '\r':														configIfs.ignore(); break;
		case '#': configIfs.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); break;
		default:
			{
				Token word;
				while (next != EOF && delimiters.find(next) == std::string::npos)
				{
					word.content += (char)configIfs.get();
					next = configIfs.peek();
				}
				tokens_.push_back(word);
				break;
			}
		}
	}
}

void Config::parseServerBlock(size_t &cursor)
{
	if (tokens_.at(cursor).content != "server" || tokens_.at(++cursor).type != Token::BlockOpen)
		throw "Unexpected token in config file";
	if (tokens_.at(cursor).content == "listen")
	{
		
	}
}

const std::vector<VirtualHost> &Config::parse(const std::string &filePath)
{
	std::ifstream configIfs;
	if (filePath.empty())
		configIfs.open(DefaultConfig);
	else
		configIfs.open(filePath);
	if (!configIfs)
		throw "Failed to open config file";
	tokenize(configIfs);
	for (size_t cursor = 0; cursor < tokens_.size(); ++cursor)
		parseServerBlock(cursor);
	return virtualHosts_;
}
