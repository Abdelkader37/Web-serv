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

		bool		done() const;

		size_t		getPosition() const;

		const Token	&peek() const;

		const std::string	&expect(Token::Type type);
		void				expect(const std::string &value);

		bool	accept(Token::Type type);
		bool	accept(const std::string &value);

		void	skipBlock();
		void	skipDirective();

		void	setPosition(size_t pos);

	private:

		std::vector<Token>	tokens_;
		size_t				pos_;
	};

	const std::vector<VirtualHost>	&parse(const std::string &filepath);

private:

	void	tokenize(std::ifstream &configIfs);

	// Directive Handlers
	void parseCgi(Route &route);
	void parseUpload(Route &route);
	void parseRedirect(Route &route);
	void parseErrorPage(Route &route);
	void parseRoot(Route &route);
	void parseIndex(Route &route);
	void parseAutoIndex(Route &route);
	void parseMaxBodySize(Route &route);
	void parseMethods(Route &route);

	// Directive parsers
	bool parseServerDirective(VirtualHost &virtualHost, Route &defaults);
	bool parseLocationDirective(Route &route);
	bool parseSharedDirective(Route &route);

	// Block parsers
	void parseLocationBlock(VirtualHost &virtualHost, Route &defaults);
	void parseServerBlock();

	TokenStream					tokenStream_;
	std::vector<VirtualHost>	virtualHosts_;

};
