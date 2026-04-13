#include "config/Config.hpp"
#include <stdexcept>

Config::TokenStream::TokenStream() : pos_(0) { }

void Config::TokenStream::push(const Token &token)		{ tokens_.push_back(token); }
void Config::TokenStream::push(const std::string &word)	{ tokens_.push_back(Token(word)); }
void Config::TokenStream::push(Token::Type type)		{ tokens_.push_back(Token(type)); }
bool Config::TokenStream::done() const					{ return pos_ >= tokens_.size(); }

const Config::TokenStream::Token &Config::TokenStream::peek() const { if (done()) throw std::runtime_error("Unexpected end of file"); return tokens_.at(pos_); }

const std::string &Config::TokenStream::expect(Token::Type type)
{
	if (done())
		throw std::runtime_error("Unexpected end of file");
	if (tokens_.at(pos_).type != type)
		throw std::runtime_error("Unexpected token type");
	return tokens_.at(pos_++).content;
}

void Config::TokenStream::expect(const std::string &value)
{
	if (done())
		throw std::runtime_error("Unexpected end of file");
	if (tokens_.at(pos_).content != value)
		throw std::runtime_error("Unexpected token: expected \"" + value + "\"");
	++pos_;
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

void Config::TokenStream::skipBlock()
{
	int depth = 0;
	while (!done())
	{
		if (tokens_.at(pos_).type == Token::BlockOpen)
			++depth;
		else if (tokens_.at(pos_).type == Token::BlockClose && depth-- == 1)
		{
			++pos_;
			return;
		}
		++pos_;
	}
	throw std::runtime_error("Unexpected end of file: unclosed block");
}

void Config::TokenStream::skipDirective()
{
	while (!done() && tokens_.at(pos_).type != Token::DirectiveDelimiter)
		++pos_;
	if (done())
		throw std::runtime_error("Unexpected end of file: missing ';'");
	++pos_;
}

size_t Config::TokenStream::getPosition() const		{ return pos_; }
void Config::TokenStream::setPosition(size_t pos)	{ pos_ = pos; }

Config::TokenStream::Token::Token() : type(DirectiveWord) { }
Config::TokenStream::Token::Token(const std::string &inputContent) : content(inputContent), type(DirectiveWord) { }
Config::TokenStream::Token::Token(Type inputType) : type(inputType) { }
