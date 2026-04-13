#include "config/Config.hpp"		// Config, parse, parseServerBlock, tokenize
#include "config/VirtualHost.hpp"

#include <cstddef>					// size_t
#include <cstdio>					// EOF
#include <limits>					// numeric_limits
#include <stdexcept>				// runtime_error

typedef void (Config::*LocationDirectiveHandler)(Route &);
typedef void (Config::*ServerDirectiveHandler)(VirtualHost &, Route &);

void Config::tokenize(std::ifstream &configIfs)
{
	static const std::string delimiters("{};\n\t ");

	int next;
	while ((next = configIfs.peek()) != EOF)
	{
		switch (next)
		{
		case ' ': case '\t': case '\n': case '\r':												configIfs.ignore();	break;
		case '{': tokenStream_.push(TokenStream::Token::BlockOpen);						configIfs.ignore();	break;
		case '}': tokenStream_.push(TokenStream::Token::BlockClose);						configIfs.ignore();	break;
		case ';': tokenStream_.push(TokenStream::Token::DirectiveDelimiter);				configIfs.ignore();	break;
		case '#': configIfs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');						break;
		default:
			TokenStream::Token word;
			while (next != EOF && delimiters.find(next) == std::string::npos)
			{
				word.content += (char)configIfs.get();
				next = configIfs.peek();
			}
			tokenStream_.push(word);
			break;
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
	while (!tokenStream_.done())
		parseServerBlock();
	if (virtualHosts_.empty())
		virtualHosts_.push_back(VirtualHost());
	return virtualHosts_;
}

void Config::parseServerBlock()
{
	tokenStream_.expect("server");
	tokenStream_.expect(TokenStream::Token::BlockOpen);

	Route		defaults;
	VirtualHost	virtualHost("defaultServerName");
	size_t		serverBlockStart = tokenStream_.getPosition();

	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
	{
		if (tokenStream_.accept("location"))
		{
			tokenStream_.expect(TokenStream::Token::DirectiveWord);
			tokenStream_.skipBlock();
		}
		else if (!parseServerDirective(virtualHost, defaults) && !parseLocationDirective(defaults))
			throw std::runtime_error("Unknown directive in server block: " + tokenStream_.peek().content);
	}

	tokenStream_.setPosition(serverBlockStart);
	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
	{
		if (tokenStream_.accept("location"))
			parseLocationBlock(virtualHost, defaults);
		else
			tokenStream_.skipDirective();
	}

	if (virtualHost.binds().empty())
		throw std::runtime_error("Server block must have a listen directive");

	if (virtualHost.routes().empty())
		virtualHost.addRoute(defaults);

	virtualHosts_.push_back(virtualHost);
}

void Config::parseLocationBlock(VirtualHost &virtualHost, Route &defaults)
{
	Route route(tokenStream_.expect(TokenStream::Token::DirectiveWord));

	tokenStream_.expect(TokenStream::Token::BlockOpen);

	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
		if (!parseLocationDirective(route))
			throw std::runtime_error("Unknown directive in location block: " + tokenStream_.peek().content);

	if (route.redirected()
		&& (!route.cgis().empty() || !route.upload().empty() ||
		route.autoIndexed() || !route.indexFiles().empty() ||
		!route.root().empty()))
		throw std::runtime_error("Redirect cannot be combined with other directives");

	if (!route.redirected())
		route = defaults;

	virtualHost.addRoute(route);
}

bool Config::parseServerDirective(VirtualHost &virtualHost, Route &defaults)
{
	static std::map<std::string, ServerDirectiveHandler> handlers;
	if (handlers.empty())
	{
		handlers["listen"]		= &Config::parseListen;
		handlers["serverName"]	= &Config::parseServerName;
		handlers["errorPage"]	= &Config::parseServerErrorPage;
	}
	std::map<std::string, ServerDirectiveHandler>::const_iterator handler = handlers.find(tokenStream_.peek().content);
	if (handler == handlers.end())
		return false;
	tokenStream_.expect(TokenStream::Token::DirectiveWord);
	(this->*handler->second)(virtualHost, defaults);
	return true;
}

bool Config::parseLocationDirective(Route &route)
{
	static std::map<std::string, LocationDirectiveHandler> handlers;
	if (handlers.empty())
	{
		handlers["cgi"]					= &Config::parseCgi;
		handlers["upload"]				= &Config::parseUpload;
		handlers["redirect"]			= &Config::parseRedirect;
		handlers["errorPage"]			= &Config::parseLocationErrorPage;
		handlers["root"]				= &Config::parseRoot;
		handlers["index"]				= &Config::parseIndex;
		handlers["autoIndex"]			= &Config::parseAutoIndex;
		handlers["clientMaxBodySize"]	= &Config::parseMaxBodySize;
		handlers["methods"]				= &Config::parseMethods;
	}
	std::map<std::string, LocationDirectiveHandler>::const_iterator handler = handlers.find(tokenStream_.peek().content);
	if (handler == handlers.end())
		return false;
	tokenStream_.expect(TokenStream::Token::DirectiveWord);
	(this->*handler->second)(route);
	return true;
}

void Config::parseServerErrorPage(VirtualHost &virtualHost, Route &defaults)
{
	const std::string &code	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	const std::string &page	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
	virtualHost.addErrorPage(code, page);
	defaults.addErrorPage(code, page);
}

void Config::parseListen(VirtualHost &virtualHost, Route &)
{
	const std::string &address	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	const std::string &port		= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	virtualHost.addBind(address, port);
	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
}
void Config::parseCgi(Route &route)
{
	const std::string &extension	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	const std::string &path			= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	route.addCgi(extension, path);
	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
}

void Config::parseRedirect(Route &route)
{
	const std::string &code	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	const std::string &page	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	route.setRedirect(code, page);
	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
}

void Config::parseLocationErrorPage(Route &route)
{
	const std::string &code	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	const std::string &page	= tokenStream_.expect(TokenStream::Token::DirectiveWord);
	route.addErrorPage(code, page);
	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
}

void Config::parseServerName(VirtualHost &virtualHost, Route &)			{ virtualHost.setName(tokenStream_.expect(TokenStream::Token::DirectiveWord));	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }

void Config::parseUpload(Route &route)									{ route.setUpload(tokenStream_.expect(TokenStream::Token::DirectiveWord));		tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseRoot(Route &route)									{ route.setRoot(tokenStream_.expect(TokenStream::Token::DirectiveWord));			tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseAutoIndex(Route &route)								{ route.setAutoIndex(tokenStream_.expect(TokenStream::Token::DirectiveWord));		tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseMaxBodySize(Route &route)								{ route.setMaxBodySize(tokenStream_.expect(TokenStream::Token::DirectiveWord));	tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }

void Config::parseMethods(Route &route)									{ route.clearMethods(); while (!tokenStream_.accept(TokenStream::Token::DirectiveDelimiter)) route.addMethod(tokenStream_.expect(TokenStream::Token::DirectiveWord)); }
void Config::parseIndex(Route &route)									{ while (!tokenStream_.accept(TokenStream::Token::DirectiveDelimiter)) route.addIndexFile(tokenStream_.expect(TokenStream::Token::DirectiveWord)); }
