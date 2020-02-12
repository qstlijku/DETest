#include "bundleFile.h"

#include "IBinaryArchive.h"
#include <SDL_assert.h>
#include "CPathID.h"

static void serializeString(IBinaryArchive& fp, std::string& str) {
	if (fp.isReading()) {
		str.clear();
		char it;
		do {
			fp.serialize(it);
			if (it != '\0')
				str.push_back(it);
		} while (it != '\0');
	} else {
		for (int i = 0; i < str.size(); ++i)
			fp.serialize(str[i]);
		fp.serializeConstant<char>('\0');
	}
}

void bundleFile::open(IBinaryArchive & fp) {
	fp.serializeConstant<uint32_t>(1114530924);

	fp.serializeConstant<uint16_t>(12804);

	uint16_t count = 1;
	fp.serialize(count);
	SDL_assert_release(count != 0);//Game Checks for this
	SDL_assert_release(count == 1);//My Sanity check

	//Skipped
	fp.serialize(unk1);

	//Skipped
	fp.serialize(unk2);

	for (uint16_t i = 0; i < count; ++i) {
		//CPathID for the string you can see
		fp.serialize(path);
		fp.serialize(type);
		fp.serialize(unk3);
		fp.serialize(offset);
		fp.serialize(size);
		serializeString(fp, fileName);
		fp.pad(16);

		SDL_assert_release(SDL_RWtell(fp.fp) == offset);
		//Read Data
		data.resize(size);
		fp.memBlock(data.data(), 1, size);
	}

	fp.pad(16);
}
