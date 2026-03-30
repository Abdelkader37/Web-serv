#include "config/Config.hpp"		// Config, parse, parseServerBlock, tokenize
#include "utils/NetworkUtils.hpp"	// resolve

#include <cstdio>					// EOF
#include <limits>					// numeric_limits
#include <stdexcept>				// runtime_error

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

void Config::parseServerBlock(size_t &i)
{
	if (tokens_.at(i).content != "server" || tokens_.at(i + 1).type != Token::BlockOpen)
		throw std::runtime_error("Unexpected token in config file");

	i += 2;
	VirtualHost host;
	while (tokens_.at(i).type != Token::BlockClose)
	{
		if (tokens_.at(i).content == "listen")
		{
			if (tokens_.at(i + 1).type == Token::DirectiveDelimiter
				|| tokens_.at(i + 2).type == Token::DirectiveDelimiter
				|| tokens_.at(i + 3).type != Token::DirectiveDelimiter)
				throw std::runtime_error("Unexpected end of directive");
			std::string &address = tokens_.at(i + 1).content;
			std::string &port = tokens_.at(i + 2).content;
			if (NetworkUtils::resolve(address, port))
				host.addBind(address, port);
			else
				throw std::runtime_error("Invalid listen directive");
			i += 4;
		}
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
		throw std::runtime_error("Failed to open config file");
	tokenize(configIfs);
	for (size_t i = 0; i < tokens_.size(); ++i)
		parseServerBlock(i);
	return virtualHosts_;
}
