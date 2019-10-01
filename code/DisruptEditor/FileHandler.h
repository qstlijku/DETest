#pragma once
#include "CPathID.h"
#include <string>
#include "Vector.h"
struct SDL_RWops;

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
	SDL_RWops* openFileHash(CPathID hash);

	SDL_RWops* openFileWrite(const std::string& path);
}
