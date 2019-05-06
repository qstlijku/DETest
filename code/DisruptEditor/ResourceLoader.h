#pragma once

#include <memory>
#include <string>

class xbgFile;
class materialFile;
class xbtFile;

void resourceLoaderThread();
void getResourceLoaderProgress(int& progress, std::string& filename);

std::shared_ptr<xbgFile> loadXBG(const char *path);
std::shared_ptr<xbgFile> loadXBG(uint32_t path);

std::shared_ptr<materialFile> loadMaterial(const char *path);

std::shared_ptr<xbtFile> loadTexture(const char *path);
