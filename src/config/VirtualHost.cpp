#include "config/VirtualHost.hpp"	// VirtualHost
#include "http/HttpStatusCodes.hpp"	// HttpStatus Codes
#include "utils/NetworkUtils.hpp"	// resolve
#include "utils/StringUtils.hpp"	// toLower
#include <string>					// string
#include <utility>					// make_pair, pair

VirtualHost::VirtualHost() : name_("default"), binds_(1, std::make_pair("127.0.0.1", "80")), routes_(1, Route())
{
	errorPages_.insert(std::make_pair(HttpStatus::BadRequest,						HtmlDir "/400.html"));
	errorPages_.insert(std::make_pair(HttpStatus::Forbidden,						HtmlDir "/403.html"));
	errorPages_.insert(std::make_pair(HttpStatus::NotFound,						HtmlDir "/404.html"));
	errorPages_.insert(std::make_pair(HttpStatus::MethodNotAllowed,				HtmlDir "/405.html"));
	errorPages_.insert(std::make_pair(HttpStatus::ContentTooLarge,				HtmlDir "/413.html"));
	errorPages_.insert(std::make_pair(HttpStatus::URITooLong,						HtmlDir "/414.html"));
	errorPages_.insert(std::make_pair(HttpStatus::IamATeapot,						HtmlDir "/418.html"));
	errorPages_.insert(std::make_pair(HttpStatus::RequestHeaderFieldsTooLarge,	HtmlDir "/431.html"));
	errorPages_.insert(std::make_pair(HttpStatus::InternalServerError,			HtmlDir "/500.html"));
	errorPages_.insert(std::make_pair(HttpStatus::NotImplemented,					HtmlDir "/501.html"));
	errorPages_.insert(std::make_pair(HttpStatus::GatewayTimeout,					HtmlDir "/504.html"));
	errorPages_.insert(std::make_pair(HttpStatus::HTTPVersionNotSupported,		HtmlDir "/505.html"));
}

void VirtualHost::addBind(const std::string &address, const std::string &port)
{
	if (!NetworkUtils::resolve(address, port))
		throw std::runtime_error("Invalid listen configuration");
	binds_.push_back(std::make_pair(address, port));
}

void VirtualHost::setName(const std::string &name)
{
	for (size_t i = 0; i < name.size(); ++i)
		if (!std::isalnum(name.at(i)) && name.at(i) != '.' && name.at(i) != '-')
			throw std::runtime_error("Invalid server name");
	name_ = StringUtils::toLower(name);
}

void VirtualHost::addRoute(Route &route) { routes_.push_back(route); }

const std::vector<std::pair<std::string, std::string> >	&VirtualHost::binds()		const { return binds_; }
const std::string										&VirtualHost::name()		const { return name_; }
const std::map<statusCode, std::string>					&VirtualHost::errorPages()	const { return errorPages_; }
const std::vector<Route>								&VirtualHost::routes()		const { return routes_; }
