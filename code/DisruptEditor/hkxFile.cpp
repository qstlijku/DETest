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

	fp.serializeConstant<uint32_t>(1474355287);//magic1
	fp.serializeConstant<uint32_t>(281067536);//magic2

	fp.serialize(userTag);//userTag
	fp.serializeConstant<uint32_t>(0x09);//fileVersion

	//This is for x64 pc
	//layoutRules
	fp.serializeConstant<uint8_t>(8);//Trying to process a binary file with a different pointer size than this platform.
	fp.serializeConstant<uint8_t>(1);//Trying to process a binary file with a different endian than this platform.
	fp.serializeConstant<uint8_t>(0);//Trying to process a binary file with a different padding optimization than this platform.
	fp.serializeConstant<uint8_t>(1);//Trying to process a binary file with a different empty base class optimization than this platform.

	fp.serializeConstant<uint32_t>(sections.size());//numSections, hardcoded
	fp.serializeConstant<uint32_t>(2);//contentsSectionIndex
	fp.serializeConstant<uint32_t>(0);//contentsSectionOffset
	fp.serializeConstant<uint32_t>(0);//contentsClassNameSectionIndex
	fp.serializeConstant<uint32_t>(75);//contentsClassNameSectionOffset

	//contentsVersion, 16 bytes
	const char* versionStr = "hk_2012.2.0-r1";
	for (const char* it = versionStr; *it != '\0'; ++it)
		fp.serializeConstant(*it);
	fp.serializeConstant<uint8_t>(0);//Null term
	fp.serializeConstant<uint8_t>(0xFF);

	//pad
	fp.serializeConstant<uint32_t>(0);
	fp.serializeConstant<uint32_t>(0xFFFFFFFF);
	//This is the end of the 64 byte header, hkPackfileHeader

	//Read Sections
	for(size_t i = 0; i < sections.size(); ++i)
		fp.serialize(sections[i]);

	//Read the first section, should be __classnames__
	SDL_assert_release(sections[CLASSNAMES].absoluteDataStart == SDL_RWtell(fp.fp) - beginOffset);
	Sint64 mark = SDL_RWtell(fp.fp) + sections[CLASSNAMES].endOffset;
	while(SDL_RWtell(fp.fp) < mark - 5) {
		auto& it = classNames.emplace_back();
		fp.serialize(it);
	}
	//Read Pad
	for (int i = 0; i < 5; ++i)
		fp.serializeConstant<uint8_t>(0xFF);

	//Make sure __types__ is empty
	SDL_assert_release(sections[TYPES].localFixupsOffset == 0);
	SDL_assert_release(sections[TYPES].globalFixupsOffset == 0);
	SDL_assert_release(sections[TYPES].virtualFixupsOffset == 0);
	SDL_assert_release(sections[TYPES].exportsOffset == 0);
	SDL_assert_release(sections[TYPES].importsOffset == 0);
	SDL_assert_release(sections[TYPES].endOffset == 0);

	//Read __data__
	SDL_assert_release(sections[DATA].absoluteDataStart == SDL_RWtell(fp.fp) - beginOffset);

	__debugbreak();
}

void hkxFile::Section::read(IBinaryArchive& fp) {
	fp.memBlock(sectionTag.data(), 1, sectionTag.size());
	fp.serializeConstant<uint8_t>(0xFF);//nullByte

	fp.serialize(absoluteDataStart);
	fp.serialize(localFixupsOffset);
	fp.serialize(globalFixupsOffset);
	fp.serialize(virtualFixupsOffset);
	fp.serialize(exportsOffset);
	fp.serialize(importsOffset);
	fp.serialize(endOffset);
}

void hkxFile::ClassNames::read(IBinaryArchive& fp) {
	fp.serialize(signature);
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
