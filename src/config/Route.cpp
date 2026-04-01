#include "config/Route.hpp"			// Route
#include "http/HttpStatusCodes.hpp"	// Code, isRedirectCode, toCode
#include <stdexcept>				// runtime_error

Route::Route()							: path_("/"), redirect_(false), root_(HtmlDir), indexFile_("welcome.html"), methods_(1, "GET"), autoindex_(false) {};
Route::Route(const std::string &path)	: path_(path), redirect_(false), autoindex_(false) {}

const std::string							&Route::path()			const { return path_; }
size_t										Route::maxBodySize()	const { return maxBodySize_; }
const std::vector<std::string>				&Route::methods()		const { return methods_; };
const std::string							&Route::root()			const { return root_; }
bool										Route::redirected()		const { return redirect_; }
const std::string							&Route::redirectPage()	const { return redirectPage_; }
HttpStatus::Code							Route::redirectCode()	const { return redirectCode_; }
bool										Route::autoIndexed()	const { return autoindex_; }
const std::string							&Route::indexFile()		const { return indexFile_; }
const std::map<std::string, std::string>	&Route::cgis()			const { return cgis_; }
const std::string							&Route::upload()		const {return upload_; }


// move validation logic from Config.cpp to here 
void	Route::setPath(const std::string &path)									{ path_ = path; }
void	Route::setMaxBodySize(size_t maxBodySize)								{ maxBodySize_ = maxBodySize; }
void	Route::addMethod(const std::string &method)								{ methods_.push_back(method); }
void	Route::setRoot(const std::string &root)									{ root_ = root; }

void Route::setRedirect(const std::string &statusCode, const std::string &page)
{
	if (!HttpStatus::isRedirectCode(statusCode))
		throw std::runtime_error("Invalid redirect code");
	if (page.empty() || (page.compare(0, 7, "http://") != 0 && page.compare(0, 8, "https://") != 0 && page.at(0) != '/'))
		throw std::runtime_error("Invalid redirect URL");
	redirect_ = true;
	redirectCode_ = HttpStatus::toCode(statusCode);
	redirectPage_ = page;
}
void	Route::setAutoIndex(bool autoindex)										{ autoindex_ = autoindex; }
void	Route::setIndexFile(const std::string &indexFile)						{ indexFile_ = indexFile; }
void	Route::addCgi(const std::string &extension, const std::string &path)	{ cgis_[extension] = path; }
void	Route::setUpload(const std::string &upload)								{ upload_ = upload; }
