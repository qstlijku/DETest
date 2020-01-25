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
	std::unique_ptr<tinyxml2::XMLDocument> gameXML;

	//Terrain Sectors
	glm::ivec2 GridsWorldOffset;
	glm::ivec2 GridMapSectorsCount;
	int GridMapSectorsGranularity;

	glm::vec3 terrainCursor;

	bool readyToRender = false;
	float loadingProgress = 1.f;
	std::string loadingStatus;

	Vector<CSector> sectors;
	std::mutex mutex;

	static void loadWLUAsync();
	static void loadSectors();

	static void loaderThread();
	std::mutex graphicMutex;
	static void onResize();

	void regenWLUCommandList();

	void drawTerrain(ID3D11DeviceContext* context);
	void regenTerrainCommandList();
	ID3D11CommandList* terrainCommandList = NULL;
};

extern World world;
