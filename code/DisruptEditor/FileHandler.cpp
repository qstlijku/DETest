#include "FileHandler.h"
#include "Common.h"
#include "Hash.h"
#include "DB.h"
#include "SDL_log.h"
#include <filesystem>

static std::string getExt(const std::filesystem::path& path) {
	const std::string name = path.filename().generic_string();
	return name.substr(name.find('.') + 1);
}

Vector<FileInfo> FH::getFileList(const std::string &dir, const std::string &extFilter) {
	std::unordered_map<std::string, FileInfo> files;

	for (const std::string &base : settings.searchPaths) {
		std::string fullPath = base + dir;
		if (!std::filesystem::exists(fullPath.c_str())) continue;

		for (auto& p : std::filesystem::directory_iterator(fullPath)) {
			std::string path = p.path().generic_string().substr(base.size());
			if (p.is_regular_file() && files.count(path) == 0) {
				std::string ext = getExt(p);
				if (extFilter.empty() || ext == extFilter) {
					FileInfo& fi = files[path];
					fi.fullPath = path;
					fi.ext = ext;
					fi.name = p.path().filename().generic_string();
				}
			}
		}
	}

	Vector<FileInfo> outFiles;
	for (auto &file : files)
		outFiles.push_back(file.second);
	return outFiles;
}

Vector<FileInfo> FH::getFileListFromAbsDir(const std::string & fullDir, const std::string & extFilter) {
	Vector<FileInfo> outFiles;
	if (!std::filesystem::exists(fullDir.c_str())) return outFiles;

	for (auto& p : std::filesystem::directory_iterator(fullDir)) {
		if (p.is_regular_file()) {
			std::string ext = getExt(p);
			if (extFilter.empty() || ext == extFilter) {
				FileInfo& fi = outFiles.emplace_back();
				fi.fullPath = p.path().generic_string();
				fi.ext = ext;
				fi.name = p.path().filename().generic_string();
			}
		}
	}

	return outFiles;
}

static int mem_close(SDL_RWops * context) {
	if (context) {
		free(context->hidden.mem.base);
		SDL_FreeRW(context);
	}
	return 0;
}

SDL_RWops * FH::openFile(const char *path) {
	std::string fullPath = getAbsoluteFilePath(path);
	SDL_RWops *fp = SDL_RWFromFile(fullPath.c_str(), "rb");
	if (!fp) return NULL;
	size_t size = SDL_RWsize(fp);
	void *data = malloc(size);
	SDL_RWread(fp, data, 1, size);
	SDL_RWclose(fp);

	if (size == 0)
		size = 1;

	fp = SDL_RWFromConstMem(data, size);
	fp->close = mem_close;

	return fp;
}

SDL_RWops* FH::openFileWrite(const std::string& path) {
	std::string fullPath = settings.patchDir + path;
	//Create parent directories
	std::size_t found = fullPath.find_last_of("/\\");
	std::string parentDir = fullPath.substr(0, found);
	bool ret = std::filesystem::create_directories(parentDir);
	SDL_assert_release(ret);

	return SDL_RWFromFile(fullPath.c_str(), "wb");
}

SDL_RWops * FH::openFile(uint32_t path) {
	SDL_RWops *fp = SDL_RWFromFile(getAbsoluteFilePath(path).c_str(), "rb");
	if (!fp) return NULL;
	size_t size = SDL_RWsize(fp);
	void *data = malloc(size);
	SDL_RWread(fp, data, 1, size);
	SDL_RWclose(fp);

	fp = SDL_RWFromConstMem(data, size);
	fp->close = mem_close;

	return fp;
}

std::string FH::getReverseFilename(FileHash hash) {
	auto it = DB::instance().getFileByHash(hash);
	if (!it) {
		char buffer[12];
		snprintf(buffer, sizeof(buffer), "_%08x", hash);
		return std::string(buffer);
	}

	return it->path;
}

static void handleUnknownPath(const std::string &base, std::unordered_map<FileHash, std::string>& unknownFiles) {
	for (auto& p : std::filesystem::directory_iterator(base)) {
		if (p.is_regular_file() && p.path().filename().generic_string().size() > 8) {
			std::string name = p.path().filename().generic_string().substr(0, 8);

			uint32_t hash = std::stoul(name.c_str(), NULL, 16);
			if (unknownFiles.count(hash) == 0) {
				unknownFiles[hash] = p.path().generic_string();
			}

		}

	}
}

static void genListOfUnknown(const std::string &path, std::unordered_map<FileHash, std::string> &unknownFiles) {
	std::string unknownPath = path + "__UNKNOWN/";
	
	if (!std::filesystem::exists(unknownPath.c_str())) return;

	for (auto& p : std::filesystem::directory_iterator(unknownPath)) {
		if (p.is_directory())
			handleUnknownPath(p.path().generic_string(), unknownFiles);
	}
}

static std::unordered_map<FileHash, std::string> unknownFileMap;

std::string FH::getAbsoluteFilePath(const char *path) {
	char fullPath[512];
	snprintf(fullPath, sizeof(fullPath), "%s%s", settings.patchDir.c_str(), path);
	if (std::filesystem::exists(fullPath))
		return fullPath;
	for (const std::string &base : settings.searchPaths) {
		snprintf(fullPath, sizeof(fullPath), "%s%s", base.c_str(), path);
		if (std::filesystem::exists(fullPath))
			return fullPath;
	}

	//Search Unknown Files
	FileHash hash = Hash::getFilenameHash(path);
	auto it = unknownFileMap.find(hash);
	if (it != unknownFileMap.end())
		return it->second;

	SDL_Log("Could not load file %s", path);

	throw 3;
	return std::string();
}

std::string FH::getAbsoluteFilePath(FileHash hash) {
	//Search Unknown Files
	auto it = unknownFileMap.find(hash);
	if (it != unknownFileMap.end())
		return it->second;

	//Lookup filename from DB
	auto itb = DB::instance().getFileByHash(hash);
	if (itb)
		return getAbsoluteFilePath(itb->path.c_str());

	SDL_Log("Could not load file %08x", hash);

	throw 3;
	return "";
}

void FH::Init() {
	genListOfUnknown(settings.patchDir, unknownFileMap);
	for (const std::string &base : settings.searchPaths)
		genListOfUnknown(base, unknownFileMap);
}
