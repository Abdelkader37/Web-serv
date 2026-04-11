#include "config/Config.hpp"		// Config, parse, parseServerBlock, tokenize

#include <cstddef>					// size_t
#include <cstdio>					// EOF
#include <limits>					// numeric_limits
#include <stdexcept>				// runtime_error

typedef void (Config::*RouteDirectiveHandler)(Route &);

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
	return virtualHosts_;
}

void Config::parseServerBlock()
{
	tokenStream_.expect("server");
	tokenStream_.expect(TokenStream::Token::BlockOpen);

	VirtualHost	virtualHost;
	Route		defaults;
	size_t		serverBlockStart = tokenStream_.getPosition();

	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
	{
		if (tokenStream_.accept("location"))
		{
			tokenStream_.expect(TokenStream::Token::DirectiveWord);
			tokenStream_.skipBlock();
		}
		else if (!parseServerDirective(virtualHost, defaults) && !parseSharedDirective(defaults))
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
	// check required directives here
	virtualHosts_.push_back(virtualHost);
}

void Config::parseLocationBlock(VirtualHost &virtualHost, Route &defaults)
{
	Route route(tokenStream_.expect(TokenStream::Token::DirectiveWord));

	tokenStream_.expect(TokenStream::Token::BlockOpen);

	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
		if (!parseLocationDirective(route) && !parseSharedDirective(route))
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
	if (tokenStream_.accept("listen"))
	{
		virtualHost.addBind(
			tokenStream_.expect(TokenStream::Token::DirectiveWord),
			tokenStream_.expect(TokenStream::Token::DirectiveWord)
		);
		tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
	}
	else if (tokenStream_.accept("serverName"))
	{
		virtualHost.setName(tokenStream_.expect(TokenStream::Token::DirectiveWord));
		tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
	}
	else if (tokenStream_.accept("errorPage"))
	{
		const std::string &code = tokenStream_.expect(TokenStream::Token::DirectiveWord);
		const std::string &page = tokenStream_.expect(TokenStream::Token::DirectiveWord);
		tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
		virtualHost.addErrorPage(code, page);
		defaults.addErrorPage(code, page);
	}
	else
		return false;
	return true;
}

bool Config::parseLocationDirective(Route &route)
{
	static std::map<std::string, RouteDirectiveHandler> handlers;
	if (handlers.empty())
	{
		handlers["cgi"]			= &Config::parseCgi;
		handlers["upload"]		= &Config::parseUpload;
		handlers["redirect"]	= &Config::parseRedirect;
		handlers["errorPage"]	= &Config::parseErrorPage;
	}
	std::map<std::string, RouteDirectiveHandler>::const_iterator handler = handlers.find(tokenStream_.peek().content);
	if (handler == handlers.end())
		return false;
	tokenStream_.expect(TokenStream::Token::DirectiveWord);
	(this->*handler->second)(route);
	return true;
}

bool Config::parseSharedDirective(Route &route)
{
	static std::map<std::string, RouteDirectiveHandler> handlers;
	if (handlers.empty())
	{
		handlers["root"]              = &Config::parseRoot;
		handlers["index"]             = &Config::parseIndex;
		handlers["autoIndex"]         = &Config::parseAutoIndex;
		handlers["clientMaxBodySize"] = &Config::parseMaxBodySize;
		handlers["methods"]           = &Config::parseMethods;
	}
	std::map<std::string, RouteDirectiveHandler>::const_iterator handler = handlers.find(tokenStream_.peek().content);
	if (handler == handlers.end())
		return false;
	tokenStream_.expect(TokenStream::Token::DirectiveWord);
	(this->*handler->second)(route);
	return true;
}

void Config::parseCgi(Route &route)			{ route.addCgi(tokenStream_.expect(TokenStream::Token::DirectiveWord), tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseUpload(Route &route)		{ route.setUpload(tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseRedirect(Route &route)	{ route.setRedirect(tokenStream_.expect(TokenStream::Token::DirectiveWord), tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseErrorPage(Route &route)	{ route.addErrorPage(tokenStream_.expect(TokenStream::Token::DirectiveWord), tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseRoot(Route &route)		{ route.setRoot(tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseAutoIndex(Route &route)	{ route.setAutoIndex(tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseMaxBodySize(Route &route)	{ route.setMaxBodySize(tokenStream_.expect(TokenStream::Token::DirectiveWord)); tokenStream_.expect(TokenStream::Token::DirectiveDelimiter); }
void Config::parseMethods(Route &route)		{ while (!tokenStream_.accept(TokenStream::Token::DirectiveDelimiter)) route.addMethod(tokenStream_.expect(TokenStream::Token::DirectiveWord)); }
void Config::parseIndex(Route &route)		{ while (!tokenStream_.accept(TokenStream::Token::DirectiveDelimiter)) route.addIndexFile(tokenStream_.expect(TokenStream::Token::DirectiveWord)); }

Config::TokenStream::Token::Token() : type(DirectiveWord) { }
Config::TokenStream::Token::Token(const std::string &inputContent) : content(inputContent), type(DirectiveWord) { }
Config::TokenStream::Token::Token(Type inputType) : type(inputType) { }

Config::TokenStream::TokenStream() : pos_(0) { }

void Config::TokenStream::push(const Token &token)		{ tokens_.push_back(token); }
void Config::TokenStream::push(const std::string &word)	{ tokens_.push_back(Token(word)); }
void Config::TokenStream::push(Token::Type type)		{ tokens_.push_back(Token(type)); }
bool Config::TokenStream::done() const					{ return pos_ >= tokens_.size(); }

const Config::TokenStream::Token &Config::TokenStream::peek() const { if (done()) throw std::runtime_error("Unexpected end of file"); return tokens_.at(pos_); }

const std::string &Config::TokenStream::expect(Token::Type type)
{
	if (done())								throw std::runtime_error("Unexpected end of file");
	if (tokens_.at(pos_).type != type)	throw std::runtime_error("Unexpected token type");
	return tokens_.at(pos_++).content;
}

void Config::TokenStream::expect(const std::string &value)
{
	if (done())									throw std::runtime_error("Unexpected end of file");
	if (tokens_.at(pos_++).content != value)	throw std::runtime_error("Unexpected token: expected \"" + value + "\"");
}

bool Config::TokenStream::accept(Token::Type type)
{
	if (done() || tokens_.at(pos_).type != type)	return false;
	++pos_;
	return true;
}

bool Config::TokenStream::accept(const std::string &value)
{
	if (done() || tokens_.at(pos_).content != value)	return false;
	++pos_;
	return true;
}

void Config::TokenStream::skipBlock()
{
	int depth = 0;
	while (!done())
	{
		if (tokens_.at(pos_).type == Token::BlockOpen)
			++depth;
		else if (tokens_.at(pos_).type == Token::BlockClose && depth-- == 1)
			return;
		++pos_;
	}
}

void Config::TokenStream::skipDirective()
{
	while (!done() && tokens_.at(pos_).type != Token::DirectiveDelimiter)
		++pos_;
	++pos_;
}

size_t Config::TokenStream::getPosition() const		{ return pos_; }
void Config::TokenStream::setPosition(size_t pos)	{ pos_ = pos; }
