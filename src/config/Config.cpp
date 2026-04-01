#include "config/Config.hpp"		// Config, parse, parseServerBlock, tokenize

#include <cstdio>					// EOF
#include <limits>					// numeric_limits
#include <stdexcept>				// runtime_error

void Config::tokenize(std::ifstream &configIfs)
{
	static const std::string delimiters("{};\n\t ");

	int next;
	while ((next = configIfs.peek()) != EOF)
	{
		switch (next)
		{
		case ' ': case '\t': case '\n': case '\r':									configIfs.ignore(); break;
		case '{': tokenStream_.push(TokenStream::Token::BlockOpen);			configIfs.ignore(); break;
		case '}': tokenStream_.push(TokenStream::Token::BlockClose);			configIfs.ignore(); break;
		case ';': tokenStream_.push(TokenStream::Token::DirectiveDelimiter);	configIfs.ignore(); break;
		case '#': configIfs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');			break;
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

void Config::parseLocationBlock(VirtualHost &virtualHost)
{
	tokenStream_.expect("location");

	Route route(tokenStream_.expect(TokenStream::Token::DirectiveWord));

	tokenStream_.expect(TokenStream::Token::BlockOpen);

	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
	{
		if (tokenStream_.accept("cgi"))
		{
			route.addCgi(
				tokenStream_.expect(TokenStream::Token::DirectiveWord), 
				tokenStream_.expect(TokenStream::Token::DirectiveWord)
			);
			tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
		}
		else if (tokenStream_.accept("upload"))
		{
			route.setUpload(tokenStream_.expect(TokenStream::Token::DirectiveWord));
			tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
		}
		else if (tokenStream_.accept("redirect"))
		{
			route.setRedirect(
				tokenStream_.expect(TokenStream::Token::DirectiveWord),
				tokenStream_.expect(TokenStream::Token::DirectiveWord)
			);
			tokenStream_.expect(TokenStream::Token::DirectiveDelimiter);
		}
		else
			throw std::runtime_error("Unknown directive in location block: " + tokenStream_.peek().content);
	}
	if (route.redirected() 
		&& (!route.cgis().empty() || !route.upload().empty() ||
		route.autoIndexed() || !route.indexFile().empty() ||
		!route.root().empty()))
		throw std::runtime_error("redirect cannot be combined with other directives");
	virtualHost.addRoute(route);
}

void Config::parseServerBlock()
{
	tokenStream_.expect("server");
	tokenStream_.expect(TokenStream::Token::BlockOpen);

	VirtualHost virtualHost;
	Route Defaults;
	while (!tokenStream_.accept(TokenStream::Token::BlockClose))
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
		// other directives here...
		else if (tokenStream_.peek().content == "location")
			parseLocationBlock(virtualHost);
		else
			throw std::runtime_error("Unknown directivein server block: " + tokenStream_.peek().content);
	}
	// check required directives here
	virtualHosts_.push_back(virtualHost);
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

// -------- Token --------
Config::TokenStream::Token::Token() : type(DirectiveWord) { }
Config::TokenStream::Token::Token(const std::string &inputContent) : content(inputContent), type(DirectiveWord) { }
Config::TokenStream::Token::Token(Type inputType) : type(inputType) { }

// -------- TokenStream --------
Config::TokenStream::TokenStream() : pos_(0) { }

void Config::TokenStream::push(const Token &token)			{ tokens_.push_back(token); }
void Config::TokenStream::push(const std::string &word)		{ tokens_.push_back(Token(word)); }
void Config::TokenStream::push(Token::Type type)			{ tokens_.push_back(Token(type)); }
bool Config::TokenStream::done() const						{ return pos_ >= tokens_.size(); }

const Config::TokenStream::Token &Config::TokenStream::peek()
{
	if (done()) throw std::runtime_error("Unexpected end of file");

	return tokens_.at(pos_);
}

const std::string &Config::TokenStream::expect(Token::Type type)
{
	if (done()) throw std::runtime_error("Unexpected end of file");

	if (tokens_.at(pos_).type != type)
		throw std::runtime_error("Unexpected token type");
	return tokens_.at(pos_++).content;
}

void Config::TokenStream::expect(const std::string &value)
{
	if (done()) throw std::runtime_error("Unexpected end of file");

	if (tokens_.at(pos_++).content != value)
		throw std::runtime_error("Unexpected token: expected \"" + value + "\"");
}

bool Config::TokenStream::accept(Token::Type type)
{
	if (done() || tokens_.at(pos_).type != type)
		return false;
	++pos_;
	return true;
}

bool Config::TokenStream::accept(const std::string &value)
{
	if (done() || tokens_.at(pos_).content != value)
		return false;
	++pos_;
	return true;
}
