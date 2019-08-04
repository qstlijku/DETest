#pragma once

#include <memory>
#include <string>

class xbgFile;
class materialFile;
class xbtFile;

std::shared_ptr<xbgFile> loadXBG(const char *path);
std::shared_ptr<xbgFile> loadXBG(uint32_t path);

std::shared_ptr<materialFile> loadMaterial(const char *path);

std::shared_ptr<xbtFile> loadTexture(const char *path);
