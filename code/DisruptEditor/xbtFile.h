#pragma once

#include "IBinaryArchive.h"
#include "DDRenderInterface.h"

class xbtFile : public Texture {
public:
	//uint32_t magic;
	//uint32_t version;

	//uint32_t offset;
	uint32_t unk;

	uint32_t u01010101_1;//01010101
	uint32_t u01010101_2;//01010101

	uint8_t unk4;
	uint8_t unk0;
	//uint8_t ;//0xFF
	//uint8_t ;//0xFF
	uint32_t unk1;

	uint32_t unk2;
	uint32_t unk3;

	std::string mipFile;
	std::vector<uint8_t> data;

	bool open(IBinaryArchive &reader);
};

