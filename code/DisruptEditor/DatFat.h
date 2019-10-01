#pragma once

#include <string>
#include <unordered_map>
#include <stdio.h>
#include <memory>
#include "CPathID.h"

struct SDL_RWops;

void InitXCompress();

class DatFat {
public:
	DatFat(const std::string &filename);
	~DatFat();
	SDL_RWops* openRead(CPathID hash);

	struct FileEntry {
		uint64_t offset;
		uint32_t realSize;
		uint32_t size;
		enum Compression {
			None = 0,
			LZO1x = 1,
			Zlib = 2,
			Xbox = 3,
		};
		Compression compression;
	};
	std::unordered_map<CPathID, FileEntry> files;

	void* file = NULL;
	void* fileMapping = NULL;
	const uint8_t* datPtr = NULL;
};

