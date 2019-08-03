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

struct ResourceLoadRequestBase {
	std::string filename;
	CPathID path;
	virtual void load() = 0;
};

template <typename T>
struct ResourceLoadRequest : public ResourceLoadRequestBase {
	std::shared_ptr<T> ptr;
	void load() {
		SDL_RWops* fp =  NULL;
		try {
			
			if (filename.empty())
				fp = FH::openFile(path.id);
			else
				fp = FH::openFile(filename.c_str());
			if (!fp)
				return;

			CBinaryArchiveReader reader(fp);
			ptr->open(reader);
			SDL_RWclose(fp);
			ptr->loaded = true;
		} catch (...) {
			if(fp)
				SDL_RWclose(fp);
		}
	}
};

static xbgMipFile loadXBGMIP(const std::string& path) {
	SDL_RWops* fp = FH::openFile(path.c_str());
	xbgMipFile model;
	if (fp) {
		model.open(CBinaryArchiveReader(fp));
		SDL_RWclose(fp);
	}
	return model;
}

template <>
struct ResourceLoadRequest<xbgFile> : public ResourceLoadRequestBase {
	std::shared_ptr<xbgFile> ptr;
	void load() {
		SDL_RWops* fp = NULL;
		try {

			if (filename.empty())
				fp = FH::openFile(path.id);
			else
				fp = FH::openFile(filename.c_str());
			if (!fp)
				return;

			CBinaryArchiveReader reader(fp);
			ptr->open(reader);
			SDL_RWclose(fp);
			if (ptr->mips.size() == 1) {
				xbgMipFile xbgmip = loadXBGMIP(ptr->mips[0].path);
				ptr->buffers.insert(ptr->buffers.begin(), xbgmip.buffers.begin(), xbgmip.buffers.end());
				ptr->mips.clear();
				ptr->unk3 = 0;
			}
			ptr->loaded = true;
		} catch (...) {
			if (fp)
				SDL_RWclose(fp);
		}
	}
};

static std::mutex mutex;
static std::queue<std::shared_ptr<ResourceLoadRequestBase>> requests;

void resourceLoaderThread() {
	while (true) {
		mutex.lock();
		bool emptyRequests = requests.empty();
		if (!emptyRequests) {
			std::shared_ptr<ResourceLoadRequestBase> request = requests.front();
			requests.pop();
			mutex.unlock();

			request->load();
		} else {
			mutex.unlock();
		}

		if (emptyRequests)
			Sleep(1);
	}
}

void getResourceLoaderProgress(int& progress, std::string& filename) {
	mutex.lock();
	progress = requests.size();
	filename.clear();
	if (!requests.empty())
		filename = requests.front()->filename;
	mutex.unlock();
}

template <typename T>
static void makeRequest(std::shared_ptr<T> ref, const char* filename) {
	auto request = std::make_shared<ResourceLoadRequest<T>>();
	request->filename = filename;
	request->ptr = ref;
	mutex.lock();
	requests.push(request);
	mutex.unlock();
}

template <typename T>
static void makeRequest(std::shared_ptr<T> ref, uint32_t hash) {
	auto request = std::make_shared<ResourceLoadRequest<T>>();
	request->path.id = hash;
	request->ptr = ref;
	mutex.lock();
	requests.push(request);
	mutex.unlock();
}

std::shared_ptr<xbgFile> loadXBG(const char *path) {
	uint32_t hash = Hash::getFilenameHash(path);
	std::shared_ptr<xbgFile> model = xbgs[hash];
	if (!model) {
		model = xbgs[hash] = std::make_shared<xbgFile>();
		makeRequest(model, path);
	}
	return model;
}

std::shared_ptr<xbgFile> loadXBG(uint32_t hash) {
	std::shared_ptr<xbgFile> model = xbgs[hash];
	if (!model) {
		model = xbgs[hash] = std::make_shared<xbgFile>();
		makeRequest(model, hash);
	}
	return model;
}

std::shared_ptr<materialFile> loadMaterial(const char *path) {
	uint32_t hash = Hash::getFilenameHash(path);
	std::shared_ptr<materialFile> model = materials[hash];
	if (!model) {
		model = materials[hash] = std::make_shared<materialFile>();
		makeRequest(model, path);
	}
	return model;
}

std::shared_ptr<xbtFile> loadTexture(const char *path) {
	std::shared_ptr<xbtFile> model = textures[path];
	if (!model) {
		model = textures[path] = std::make_shared<xbtFile>();
		makeRequest(model, path);
	}
	return model;
}
