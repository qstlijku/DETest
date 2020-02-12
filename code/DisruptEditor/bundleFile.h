#pragma once

class IBinaryArchive;
#include "CPathID.h"
#include "CStringID.h"
#include <string>
#include <vector>

class bundleFile {
public:
	uint32_t unk1;
	uint32_t unk2;

	//struct bundledFile {
		CPathID path;
		CStringID type;
		uint32_t unk3;
		uint32_t offset;
		uint32_t size;
		std::string fileName;
		std::vector<uint8_t> data;
	//};

	void open(IBinaryArchive &fp);
};

