#pragma once

#include <memory>
#include <map>
#include "tinyxml2.h"
#include "wluFile.h"
#include "CSector.h"

class World {
public:
	std::unique_ptr<tinyxml2::XMLDocument> particles;
	std::unique_ptr<tinyxml2::XMLDocument> spawnPointList;
	std::map<std::string, std::shared_ptr<wluFile> > wlus;
	float loadingProgress = 1.f;
	std::string loadingStatus;

	Vector<CSector> sectors;

	static void loadWLUAsync();
	static void loadSectors();
};

extern World world;
