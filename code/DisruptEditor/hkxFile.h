#pragma once

#include <array>
#include <vector>

class IBinaryArchive;

class hkxFile {
public:
	uint32_t unk1;

	struct Section {
		std::array<char, 16> name;

		std::array<uint32_t, 7> offsets;

		void read(IBinaryArchive& fp);
	};
	std::vector<Section> sections;

	struct ClassNames {
		uint32_t unk1;
		std::string name;
		void read(IBinaryArchive& fp);
	};
	std::vector<ClassNames> classNames;

	void read(IBinaryArchive& fp);
};

#include <batchFile.h>

class batchCollisionFile {
public:
	batchFile::batchHeader head;
	uint32_t hkxSize;
	hkxFile hkx;

	bool open(IBinaryArchive& fp);
};

class physResourceFile {
public:
	uint32_t unk2;
	uint32_t hkxSize;
	uint32_t switchCase;

	hkxFile hkx;

	bool open(IBinaryArchive& fp);
};

