#include "World.h"

#include "FileHandler.h"
#include "IBinaryArchive.h"
#include <SDL.h>

World world;

void World::loadWLUAsync() {
	Vector<FileInfo> files = FH::getFileList("worlds/windy_city/generated/wlu", "xml.data.fcb");
	int i = 0;
	for (FileInfo& file : files) {
		
		std::shared_ptr<wluFile> wlu = std::make_shared<wluFile>();
		wlu->shortName = file.name;
		SDL_RWops* fp = FH::openFile(file.fullPath.c_str());
		wlu->open(fp);
		SDL_RWclose(fp);
		world.mutex.lock();
		world.wlus[file.name] = wlu;
		world.loadingStatus = "Loading " + file.name;
		++i;
		world.loadingProgress = (float)i / files.size();
		world.mutex.unlock();
	}
}

void World::loadBatchAsync() {
	Vector<FileInfo> files = FH::getFileList("worlds/windy_city/generated/batchmeshentity/", "cbatch");
	int i = 0;
	for (auto it : files) {
		if (it.name.find("_compound") == std::string::npos) continue;


		std::shared_ptr<batchFile> bf = std::make_shared<batchFile>();
		try {
			SDL_RWops * fp = FH::openFile(it.fullPath.c_str());
			CBinaryArchiveReader reader(fp);
			bf->open(reader);
			SDL_RWclose(fp);
			world.mutex.lock();
			world.batches[it.name] = bf;
			world.loadingStatus = "Loading " + it.name;
			world.mutex.unlock();
		} catch (...) {}
	}
}

void World::loadSectors() {
	//This is hardcoded!
	world.sectors.reserve(64 * 80);
	for (uint32_t x = 0; x < 64; x += 4) {
		for (uint32_t y = 0; y < 80; y += 4) {
			uint32_t offset = (y * 64) + x;
			SDL_Log("Loading Sector %u", offset);

			CSector sector;
			sector.xPos = x;
			sector.yPos = y;
			sector.sectorID = offset;

			char filename[80];
			snprintf(filename, sizeof(filename), "worlds/windy_city/generated/sdat/sd%u.sdat", offset);
			SDL_RWops* fp = FH::openFile(filename);
			SDL_assert_release(fp);
			CBinaryArchiveReader reader(fp);
			sector.open(reader);
			SDL_RWclose(fp);

			world.sectors.push_back(sector);
		}
	}
}
