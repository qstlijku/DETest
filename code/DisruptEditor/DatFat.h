#pragma once

#include <string>
#include <unordered_map>
#include <stdio.h>
#include <memory>
#include <Windows.h>

struct SDL_RWops;

class DatFat {
public:
	DatFat(const std::string &filename);
	~DatFat();
	SDL_RWops* openRead(uint32_t hash);

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
	std::unordered_map<uint32_t, FileEntry> files;

	HANDLE file = NULL;
	HANDLE fileMapping = NULL;
	const uint8_t* datPtr = NULL;
};

