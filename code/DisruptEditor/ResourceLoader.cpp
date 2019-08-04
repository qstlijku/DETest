#include "ResourceLoader.h"

#include "DDRenderInterface.h"
#include "IBinaryArchive.h"
#include "FileHandler.h"
#include "Hash.h"
#include "materialFile.h"
#include "xbgFile.h"
#include "xbgMipFile.h"
#include "xbtFile.h"
#include <mutex>
#include <queue>
#include <unordered_map>

static std::unordered_map<uint32_t, std::shared_ptr<xbgFile>> xbgs;
static std::unordered_map<uint32_t, std::shared_ptr<materialFile>> materials;
static std::unordered_map<std::string, std::shared_ptr<xbtFile>> textures;

static xbgMipFile loadXBGMIP(const std::string& path) {
	SDL_RWops* fp = FH::openFile(path.c_str());
	xbgMipFile model;
	if (fp) {
		model.open(CBinaryArchiveReader(fp));
		SDL_RWclose(fp);
	}
	return model;
}

std::shared_ptr<xbgFile> loadXBG(const char *path) {
	uint32_t hash = Hash::getFilenameHash(path);
	std::shared_ptr<xbgFile> model = xbgs[hash];
	if (!model) {
		model = xbgs[hash] = std::make_shared<xbgFile>();
		SDL_RWops* fp = FH::openFile(path);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
		model->loaded = true;
	}
	return model;
}

std::shared_ptr<xbgFile> loadXBG(uint32_t hash) {
	std::shared_ptr<xbgFile> model = xbgs[hash];
	if (!model) {
		model = xbgs[hash] = std::make_shared<xbgFile>();
		SDL_RWops* fp = FH::openFile(hash);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
		model->loaded = true;
	}
	return model;
}

std::shared_ptr<materialFile> loadMaterial(const char *path) {
	uint32_t hash = Hash::getFilenameHash(path);
	std::shared_ptr<materialFile> model = materials[hash];
	if (!model) {
		model = materials[hash] = std::make_shared<materialFile>();
		SDL_RWops* fp = FH::openFile(path);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
		model->loaded = true;
	}
	return model;
}

std::shared_ptr<xbtFile> loadTexture(const char *path) {
	std::shared_ptr<xbtFile> model = textures[path];
	if (!model) {
		model = textures[path] = std::make_shared<xbtFile>();
		SDL_RWops* fp = FH::openFile(path);
		if (!fp)
			return model;

		CBinaryArchiveReader reader(fp);
		model->open(reader);
		SDL_RWclose(fp);
		model->loaded = true;
	}
	return model;
}
