#pragma once

#include <stdint.h>

class IBinaryArchive;
#include <vector>

class mabFile {
public:
	void open(IBinaryArchive &fp);

	// TODO: fix these later
	uint32_t header1;
	uint32_t header2;
	uint32_t header3;

	uint8_t signature[3];
	uint8_t flags;

	int animationDataSize;
	float duration;
	float animFrameRate;
	uint16_t numBonesInAnim;
	// WD1, come back to this later
	uint16_t dataCounts1[7];
	uint32_t offsets1[11];

	uint16_t dataCounts[10];
	uint32_t offsets[12];

	uint16_t lastFrame;

	std::vector<uint16_t> boneHashes;
	std::vector<uint16_t> frameArray; // unkArrayD in template

	std::vector<uint32_t> jointRotOffsets;
	std::vector<uint8_t> bitstream;
};
