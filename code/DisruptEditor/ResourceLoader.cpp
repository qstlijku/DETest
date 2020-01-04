#include "ResourceLoader.h"

#include "DDRenderInterface.h"
#include "IBinaryArchive.h"
#include "FileHandler.h"
#include "Hash.h"
#include "materialFile.h"
#include "xbgFile.h"
#include "xbgMipFile.h"
#include "xbtFile.h"
#include "SplineLoft.h"
#include "batchFile.h"
#include "buildingBatchFile.h"
#include <mutex>
#include <queue>
#include <unordered_map>
#include <SDL_log.h>

static std::unordered_map<CPathID, std::shared_ptr<xbgFile>> xbgs;
static std::unordered_map<CPathID, std::shared_ptr<materialFile>> materials;
static std::unordered_map<CPathID, std::shared_ptr<xbtFile>> textures;
static std::unordered_map<CPathID, std::shared_ptr<SplineLoftHiRes>> hiResSplineLofts;
static std::unordered_map<CPathID, std::shared_ptr<batchFile>> batches;
static std::unordered_map<CPathID, std::shared_ptr<buildingBatchFile>> buildingBatches;

static xbgMipFile loadXBGMIP(const std::string& path) {
	SDL_RWops* fp = FH::openFile(path.c_str());
	xbgMipFile model;
	if (fp) {
		CBinaryArchiveReader reader(fp);
		model.open(reader);
		SDL_RWclose(fp);
	}
	return model;
}

std::shared_ptr<xbgFile> loadXBG(CPathID hash) {
	std::shared_ptr<xbgFile> model = xbgs[hash];
	if (!model) {
		model = xbgs[hash] = std::make_shared<xbgFile>();
		SDL_RWops* fp = FH::openFileHash(hash);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);

		if (model->mips.size() == 1) {
			xbgMipFile xbgmip = loadXBGMIP(model->mips[0].path);
			model->buffers.insert(model->buffers.begin(), xbgmip.buffers.begin(), xbgmip.buffers.end());
			model->mips.clear();
			model->unk3 = 0;
		}
		
		//Create Buffers
		for (auto& it : model->buffers)
			it.createBuffers();
	}
	return model;
}

std::shared_ptr<materialFile> loadMaterial(CPathID path) {
	std::shared_ptr<materialFile> model = materials[path];
	if (!model) {
		model = materials[path] = std::make_shared<materialFile>();
		SDL_RWops* fp = FH::openFileHash(path);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
	}
	return model;
}

std::shared_ptr<xbtFile> loadTexture(CPathID path) {
	std::shared_ptr<xbtFile> model = textures[path];
	if (!model) {
		model = textures[path] = std::make_shared<xbtFile>();
		SDL_RWops* fp = FH::openFileHash(path);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
	}
	return model;
}

std::shared_ptr<SplineLoftHiRes> loadHiResSplineLoft(CPathID path) {
	std::shared_ptr<SplineLoftHiRes> model = hiResSplineLofts[path];
	if (!model) {
		model = hiResSplineLofts[path] = std::make_shared<SplineLoftHiRes>();
		SDL_RWops* fp = FH::openFileHash(path.id);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
	}
	return model;
}

std::shared_ptr<batchFile> loadbatchFile(CPathID path) {
	std::shared_ptr<batchFile> model = batches[path];
	if (!model) {
		model = batches[path] = std::make_shared<batchFile>();
		SDL_RWops* fp = FH::openFileHash(path.id);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
	}
	return model;
}

std::shared_ptr<buildingBatchFile> loadBuildingBatchFile(CPathID path) {
	std::shared_ptr<buildingBatchFile> model = buildingBatches[path];
	if (!model) {
		model = buildingBatches[path] = std::make_shared<buildingBatchFile>();
		SDL_RWops* fp = FH::openFileHash(path.id);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
	}
	return model;
}
