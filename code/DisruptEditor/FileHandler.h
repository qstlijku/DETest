#pragma once
#include <string>
#include "Vector.h"
struct SDL_RWops;

typedef uint32_t FileHash;

struct FileInfo {
	std::string fullPath;
	std::string name;
	std::string ext;
};

namespace FH {
	void Init();
	void AddDatFat(const std::string &filename);
	Vector<FileInfo> getFileList(const std::string &dir, const std::string &extFilter = std::string());
	Vector<FileInfo> getFileListFromAbsDir(const std::string &dir, const std::string &extFilter = std::string());

	SDL_RWops* openFile(const char *path);
	SDL_RWops* openFile(FileHash hash);

	SDL_RWops* openFileWrite(const std::string& path);
}
