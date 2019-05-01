#pragma once

#include <stdint.h>

class IBinaryArchive;

class mabFile {
public:
	void open(IBinaryArchive &fp);

	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk3;

	uint8_t unk4;
};

