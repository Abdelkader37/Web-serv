#pragma once

#include <vector>
#include <../config/Config.hpp>
#include <../http/HttpRequest.hpp>

class Cgi
{
    private:
    std::vector<std::string> env_string;
    std::vector<std::string> arg;
};

class CgiResponde
{

};

void cgi(HttpRequest request, Config config);