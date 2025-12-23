#include <cstdio>

#include "util.hpp"

void printInfo(const char *info) { std::fputs(info, stdout); }
void printError(const char *error) { std::fputs(error, stderr); }
