#pragma once

#include <memory>
#include <map>
#include <mutex>
#include "tinyxml2.h"
#include "wluFile.h"
#include "CSector.h"
#include "batchFile.h"

class World {
public:
	std::unique_ptr<tinyxml2::XMLDocument> particles;
	std::unique_ptr<tinyxml2::XMLDocument> spawnPointList;
	std::map<std::string, std::shared_ptr<wluFile> > wlus;
	std::map<std::string, std::shared_ptr<batchFile> > batches;

	bool readyToRender = false;
	float loadingProgress = 1.f;
	std::string loadingStatus;

	Vector<CSector> sectors;
	std::mutex mutex;

	static void loadWLUAsync();
	static void loadSectors();

	static void loaderThread();

	void drawTerrain(ID3D11DeviceContext* context);

	ID3D11CommandList* pd3dCommandList = NULL;
	std::vector< ID3D11CommandList* > wluLists;
};

extern World world;
