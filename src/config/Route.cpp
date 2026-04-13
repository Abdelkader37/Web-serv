#include "config/Route.hpp"			// Route
#include "http/HttpStatusCodes.hpp"	// Code, isRedirectCode, toCode
#include "utils/StringUtils.hpp"	// isAllDigits
#include <stdexcept>				// runtime_error

Route::Route()							: maxBodySize_(0), path_("/"), methods_(1, "GET"), root_(HtmlDir), redirect_(false), autoindex_(false), indexFiles_(1, "welcome.html") { }

Route::Route(const std::string &path) : maxBodySize_(0), path_(path), redirect_(false), autoindex_(false) {}

Route &Route::operator=(const Route &other)
{
	if (root_.empty())			root_			= other.root_;
	if (indexFiles_.empty())	indexFiles_		= other.indexFiles_;
	if (methods_.empty())		methods_		= other.methods_;
	if (!autoindex_)			autoindex_		= other.autoindex_;
	if (!maxBodySize_)			maxBodySize_	= other.maxBodySize_;
	if (upload_.empty())		upload_			= other.upload_;
	if (cgis_.empty())			cgis_			= other.cgis_;

	for (std::map<HttpStatus::Code, std::string>::const_iterator it = other.errorPages_.begin(); it != other.errorPages_.end(); ++it)
		errorPages_.insert(*it); // won't override

	return *this;
}

void Route::setMaxBodySize(const std::string &maxBodySize)
{
	if (!StringUtils::isAllDigits(maxBodySize))
		throw std::runtime_error("clientMaxBodySize must be a number");
	maxBodySize_ = std::strtoul(maxBodySize.c_str(), NULL, 10);
}
void	Route::setPath(const std::string &path)									{ path_ = path; }
void	Route::setRoot(const std::string &root)									{ root_ = root; }


void Route::addErrorPage(const std::string &code, const std::string &page)
{
	if (!HttpStatus::isErrorCode(code))
		throw std::runtime_error("Invalid error code");
	errorPages_[HttpStatus::toCode(code)] = page;
}

void Route::addMethod(const std::string &method)
{
	if (method != "GET" && method != "POST" && method != "DELETE")
		throw std::runtime_error("Invalid method: " + method);
	methods_.push_back(method);
}

void Route::setRedirect(const std::string &statusCode, const std::string &page)
{
	if (!HttpStatus::isRedirectCode(statusCode))
		throw std::runtime_error("Invalid redirect code");
	redirect_		= true;
	redirectCode_	= HttpStatus::toCode(statusCode);
	redirectPage_	= page;
}

void Route::setAutoIndex(const std::string &autoindex)
{
	if (autoindex == "on")			autoindex_ = true;
	else if (autoindex == "off")	autoindex_ = false;
	else							throw std::runtime_error("autoIndex must be 'on' or 'off'");
}

void	Route::addIndexFile(const std::string &indexFile)						{ indexFiles_.push_back(indexFile); }
void	Route::addCgi(const std::string &extension, const std::string &path)	{ cgis_[extension] = path; }
void	Route::setUpload(const std::string &upload)								{ upload_ = upload; }
void	Route::clearMethods() { methods_.clear(); }

const std::string								&Route::path()			const { return path_; }
size_t											Route::maxBodySize()	const { return maxBodySize_; }
const std::vector<std::string>					&Route::methods()		const { return methods_; };
const std::string								&Route::root()			const { return root_; }
bool											Route::redirected()		const { return redirect_; }
const std::string								&Route::redirectPage()	const { return redirectPage_; }
HttpStatus::Code								Route::redirectCode()	const { return redirectCode_; }
bool											Route::autoIndexed()	const { return autoindex_; }
const std::vector<std::string>					&Route::indexFiles()	const { return indexFiles_; }
const std::map<HttpStatus::Code, std::string>	&Route::errorPages()	const { return errorPages_; }
const std::map<std::string, std::string>		&Route::cgis()			const { return cgis_; }
const std::string								&Route::upload()		const { return upload_; }
