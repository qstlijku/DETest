#pragma once

#include <memory>
#include "CPathID.h"

class xbgFile;
class materialFile;
class xbtFile;

std::shared_ptr<xbgFile> loadXBG(CPathID path);

std::shared_ptr<materialFile> loadMaterial(const char *path);

std::shared_ptr<xbtFile> loadTexture(const char *path);
