#ifndef ENV_LOADER_H
#define ENV_LOADER_H

#include <string>

bool loadEnvFile(const std::string& path);
bool loadEnvFileWithFallback(const std::string& fileName);

#endif
