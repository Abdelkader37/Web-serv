#include "config/Route.hpp"			// Route

Route::Route() : path_("/"), redirect_(false), root_(HtmlDir), indexFile_("welcome.html"), methods_(1, "GET"), autoindex_(false) {};

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
const std::string							&Route::uploadDir()		const {return uploadDir_; }
