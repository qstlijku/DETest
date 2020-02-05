#include "hkxFile.h"

#include "IBinaryArchive.h"

static void serializeHkString(IBinaryArchive& fp, std::string& str) {
	if (fp.isReading()) {
		str.clear();
		char it;
		do {
			fp.serialize(it);
			if (it != '\0')
				str.push_back(it);
		} while (it != '\0');
	} else {
		SDL_assert_release(false);
	}
}

bool batchCollisionFile::open(IBinaryArchive& fp) {
	fp.serialize(head);
	if (head.size != 0) {
		fp.serialize(hkxSize);
		fp.pad(16);
		if (hkxSize != 0)
			fp.serialize(hkx);
	}
	return true;
}

void hkxFile::read(IBinaryArchive& fp) {
	return;

	fp.padding = fp.PADDING_NONE;
	size_t beginOffset = SDL_RWtell(fp.fp);

	fp.serializeConstant<uint32_t>(1474355287);
	fp.serializeConstant<uint32_t>(281067536);

	fp.serialize(unk1);
	fp.serializeConstant<uint32_t>(0x09);

	//This is for x64 pc
	fp.serializeConstant<uint8_t>(8);//Trying to process a binary file with a different pointer size than this platform.
	fp.serializeConstant<uint8_t>(1);//Trying to process a binary file with a different endian than this platform.
	fp.serializeConstant<uint8_t>(0);//Trying to process a binary file with a different padding optimization than this platform.
	fp.serializeConstant<uint8_t>(1);//Trying to process a binary file with a different empty base class optimization than this platform.

	fp.serializeConstant<uint32_t>(3);//Number of sections, hardcoded to 3
	fp.serializeConstant<uint32_t>(2);
	fp.serializeConstant<uint32_t>(0);
	fp.serializeConstant<uint32_t>(0);
	fp.serializeConstant<uint32_t>(75);

	const char* versionStr = "hk_2012.2.0-r1";
	for (const char* it = versionStr; *it != '\0'; ++it)
		fp.serializeConstant(*it);
	fp.serializeConstant<uint8_t>(0);//Null term

	//pad this to the next 4bytes
	fp.serializeConstant<uint8_t>(0xFF);

	fp.serializeConstant<uint32_t>(0);
	fp.serializeConstant<uint32_t>(0xFFFFFFFF);
	//This is the end of the 64 byte header

	sections.resize(3);
	for(size_t i = 0; i < sections.size(); ++i)
		fp.serialize(sections[i]);

	//Read the first section, should be __classnames__
	SDL_assert_release(sections[0].offsets[0] == SDL_RWtell(fp.fp) - beginOffset);
	Sint64 mark = SDL_RWtell(fp.fp) + sections[0].offsets[1];
	while(SDL_RWtell(fp.fp) < mark) {
		auto& it = classNames.emplace_back();
		fp.serialize(it);
	}


}

void hkxFile::Section::read(IBinaryArchive& fp) {
	fp.memBlock(name.data(), 1, name.size());
	fp.serializeConstant<uint32_t>(0xFF000000);

	for (size_t i = 0; i < offsets.size(); ++i)
		fp.serialize(offsets[i]);
}

void hkxFile::ClassNames::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serializeConstant<uint8_t>(9);
	serializeHkString(fp, name);
}

bool physResourceFile::open(IBinaryArchive& fp) {
	fp.serializeConstant<uint32_t>(0x67);
	fp.serialize(unk2);
	fp.serialize(hkxSize);
	fp.serialize(switchCase);

	return false;
}
