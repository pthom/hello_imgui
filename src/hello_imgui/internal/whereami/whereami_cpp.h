#pragma once
#ifndef __EMSCRIPTEN__
#include <string>

std::string wai_getExecutablePath_string();
std::string wai_getExecutableFolder_string();

std::string wai_getModulePath_string();
#endif