#include "locFile.h"

//Original Implementation from: https://github.com/ahmet-celik

#include <algorithm>
#include <SDL.h>
#include <string>
#include <stdio.h>
#include <stdint.h>
#include "IBinaryArchive.h"
#include "Serialization.h"

struct Id {

};

struct SubTableMeta {
	uint16_t first;
	uint16_t second;

	uint32_t deltaID;
	uint32_t size;
	uint32_t maxID;

	void read(IBinaryArchive& fp) {
		fp.serialize(first);
		fp.serialize(second);

		uint32_t whole = (first << 16) + second;
		deltaID = second;
		if (whole >= 0x80000000) {
			fp.serialize(size);
			if (((whole >> 30) & 1) != 0) {
				uint16_t extra;
				fp.serialize(extra);
				deltaID += (extra << 16);
			}
			maxID = first & 0x3FFF;
		} else {
			maxID = first >> 7;
			size = (whole >> 12) & 0x7FF;
			deltaID &= 0xFFF;
		}
	}
};

struct SubTableIds {
	void read(IBinaryArchive& fp) {
	}
};

struct Table {
	uint32_t firstID;
	uint32_t offsetLength;
	std::vector<SubTableMeta> subTables;
	std::vector<SubTableIds> subTablesIDs;

	void read(IBinaryArchive& fp) {
		fp.serialize(firstID);
		fp.serialize(offsetLength);
	}

	void read2(IBinaryArchive& fp) {
		uint32_t length = offsetLength & 0xF;

		subTables.resize(length);
		for (auto& it : subTables)
			it.read(fp);

		subTablesIDs.resize(length);
		for (auto& it : subTablesIDs)
			it.read(fp);
	}
};

void locFile::open(IBinaryArchive& fp) {
	fp.padding = fp.PADDING_NONE;

	int16_t magic = 0x4C53;
	fp.serialize(magic);
	SDL_assert_release(magic == 0x4C53);

	int16_t version = 1;
	fp.serialize(version);
	SDL_assert_release(version == 1);

	fp.serialize(language);

	int16_t tableLength;
	fp.serialize(tableLength);

	uint32_t treeOffset;
	fp.serialize(treeOffset);

	std::vector<Table> tables(tableLength);
	for (int16_t i = 0; i < tableLength; ++i)
		tables[i].read(fp);

	for (int16_t i = 0; i < tableLength; ++i)
		tables[i].read2(fp);

	uint32_t end_of_tables = 0;
	uint32_t tree_meta_offset = (uint32_t)(end_of_tables + (-end_of_tables & 3));
	uint32_t tree_meta_length = (treeOffset - tree_meta_offset) >> 2;

	std::vector<uint32_t> treeMeta();
}

void locFile::registerMembers(MemberStructure& ms) {
	REGISTER_MEMBER(language);
	REGISTER_MEMBER(uncompiledStrings);
}
