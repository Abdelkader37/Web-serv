#include "config/Route.hpp"			// Route

Route::Route() : path_("/"), redirect_(false), root_("/usr/local/webserv/html"), indexFile_("welcome.html"), methods_(1, "GET"), autoindex_(false) {};