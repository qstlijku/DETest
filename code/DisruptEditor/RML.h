#pragma once

#include "tinyxml2.h"
#include <memory>
#include <string>

struct SDL_RWops;

std::unique_ptr<tinyxml2::XMLDocument> loadRml(SDL_RWops *fp);

std::unique_ptr<tinyxml2::XMLDocument> loadXml(SDL_RWops *fp);
std::string XMLToString(tinyxml2::XMLDocument &doc);
