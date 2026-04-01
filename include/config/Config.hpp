#pragma once

#include "config/VirtualHost.hpp"	// VirtualHost


#include <string>					// string
#include <vector>					// vector
#include <fstream>					// ifstream


#define DefaultConfig InstallDir "/conf/webserv.conf"




class Config
{

public:

	class TokenStream
	{
	public:

		class Token
		{
		public:

			enum Type
			{
				DirectiveWord,
				DirectiveDelimiter,
				BlockOpen,
				BlockClose
			};

			Token();
			Token(const std::string &inputContent);
			Token(Type inputType);

			std::string	content;
			Type		type;
		};

		TokenStream();

		void	push(const Token &token);
		void	push(const std::string &word);
		void	push(Token::Type type);

		bool done() const;

		const Token &peek();

		const std::string	&expect(Token::Type type);
		void				expect(const std::string &value);

		bool	accept(Token::Type type);
		bool	accept(const std::string &value);

	private:

		std::vector<Token>	tokens_;
		size_t				pos_;
	};

	const std::vector<VirtualHost> &parse(const std::string &filepath);

private:

void	tokenize(std::ifstream &configIfs);
void	parseServerBlock();
void	parseLocationBlock(VirtualHost &virtualHost);

	TokenStream					tokenStream_;
	std::vector<VirtualHost>	virtualHosts_;

};
