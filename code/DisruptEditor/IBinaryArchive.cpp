#include "IBinaryArchive.h"
#include "FileHandler.h"
#include <SDL_log.h>
#include "Common.h"

template <typename T>
static void serializePOD(IBinaryArchive &fp, T &value) {
	if (fp.bigEndian) {
		uint8_t *it = (uint8_t*)(&value + 1) - 1;
		while (it >= (uint8_t*)&value) {
			fp.memBlock(it, 1, 1);
			--it;
		}
	} else {
		fp.memBlock(&value, sizeof(value), 1);
	}
}

IBinaryArchive::IBinaryArchive() {
	fp = NULL;
}

void IBinaryArchive::serialize(bool& value) {
	if (padding == PADDING_GEAR)
		pad(4);
	serializePOD(*this, value);
	SDL_assert_release(value == 0 || value == 1);
	if (padding == PADDING_GEAR)
		pad(4);
}

void IBinaryArchive::serialize(uint8_t& value) {
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(char& value) {
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(uint16_t& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(2);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(int16_t& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(2);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(uint32_t& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(4);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(int32_t& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(4);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(uint64_t& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(8);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(int64_t& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(8);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(float& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(4);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(double& value) {
	if (padding == PADDING_IBINARYARCHIVE || padding == PADDING_GEAR)
		pad(8);
	serializePOD(*this, value);
}

void IBinaryArchive::serialize(glm::vec2& value) {
	serialize(value.x);
	serialize(value.y);
}

void IBinaryArchive::serialize(glm::ivec2& value) {
	serialize(value.x);
	serialize(value.y);
}

void IBinaryArchive::serialize(glm::vec3& value) {
	serialize(value.x);
	serialize(value.y);
	serialize(value.z);
}

void IBinaryArchive::serialize(glm::vec4& value) {
	serialize(value.x);
	serialize(value.y);
	serialize(value.z);
	serialize(value.w);
}

void IBinaryArchive::serialize(glm::mat4& value) {
	for (int x = 0; x < 4; ++x)
		for (int y = 0; y < 4; ++y)
			serialize(value[x][y]);
}

void IBinaryArchive::serialize(std::string& value) {
	uint32_t len = (uint32_t)value.size();
	serialize(len);

	if (isReading()) {
		std::vector<char> data(len + 1);
		memBlock(data.data(), 1, len);
		value = data.data();
	} else {
		memBlock(value.data(), 1, value.size());
	}
}

void IBinaryArchive::PreAllocateSizeOfType(CStringID type, uint32_t count) {
	serializeConstant(type);

	uint32_t index = 0;

	if (!isReading()) {
		bool found = false;
		for (auto& it : head8) {
			if (it.type == type) {
				found = true;
				it.numInstances += count;
				break;
			}
			++index;
		}

		if (!found) {
			auto& it = head8.emplace_back();
			it.type = type;
			it.numInstances = count;

			//TODO
			SDL_assert_release(false);
		}
	}

	serialize(index);
	serializeConstant(count);
}

void IBinaryArchive::PreAllocatePointers(uint32_t count) {
	//I think this does nothing
}

void IBinaryArchive::PreAllocateDynamicType(CStringID type) {
	//Reads something...not sure
	uint32_t a = 0;
	serialize(a);
	a *= 0x10;
}

void IBinaryArchive::PreAllocateMemory(uint32_t unk1, uint32_t unk2) {
	serializeConstant(unk1);
	serializeConstant(unk2);
}

void IBinaryArchive::PauseInPlace() {
	numPausedInPlace++;
}

void IBinaryArchive::ResumeInPlace() {
	numPausedInPlace--;
	SDL_assert_release(numPausedInPlace >= 0);
}

void IBinaryArchive::pad(size_t padding) {
	size_t seek = getPadSize(padding);

	uint8_t data[64] = { 0 };
	memBlock(data, 1, seek);
	for (Sint64 i = 0; i < seek; ++i)
		SDL_assert_release(data[i] == 0);
}

void IBinaryArchive::padInPlace(size_t padding) {
	size_t next = inPlaceRead + padding - 1 & ~(padding - 1);
	size_t seek = next - inPlaceRead;

	uint8_t data[64] = { 0 };
	memBlockInPlace(data, 1, seek);
	for (Sint64 i = 0; i < seek; ++i)
		SDL_assert_release(data[i] == 0);
}

size_t IBinaryArchive::size() {
	return SDL_RWsize(fp);
}

size_t IBinaryArchive::tell() {
	return SDL_RWtell(fp);
}

size_t IBinaryArchive::getPadSize(size_t padding) {
	size_t size = SDL_RWtell(fp);
	size_t seek = (padding - (size % padding)) % padding;
	return seek;
}

void IBinaryArchive::Header::read(IBinaryArchive& fp) {
	fp.serialize(unk1);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);
}

void IBinaryArchive::Unk::read(IBinaryArchive& fp) {
	fp.serialize(type);
	fp.serialize(numInstances);
	fp.serialize(unk3);
	fp.serialize(unk4);
}

CBinaryArchiveReader::CBinaryArchiveReader(SDL_RWops* _fp) {
	fp = _fp;
	bigEndian = settings.bigEndian;
}

bool CBinaryArchiveReader::isReading() const {
	return true;
}

void CBinaryArchiveReader::memBlock(void* ptr, size_t objSize, size_t objCount) {
	size_t ret = SDL_RWread(fp, ptr, objSize, objCount);
	SDL_assert_release(ret == objCount);
	offset = SDL_RWtell(fp);
}

void CBinaryArchiveReader::memBlockInPlace(void* ptr, size_t objSize, size_t objCount) {
	if (numPausedInPlace) {
		memBlock(ptr, objSize, objCount);
	} else {
		SDL_assert_release(inPlaceOffset >= 0);
		Sint64 curOffset = SDL_RWtell(fp);
		SDL_RWseek(fp, inPlaceOffset, RW_SEEK_SET);
		SDL_RWread(fp, ptr, objSize, objCount);
		inPlaceOffset += objSize * objCount;
		SDL_RWseek(fp, curOffset, RW_SEEK_SET);

		inPlaceRead += objSize * objCount;
	}
}

void CBinaryArchiveReader::markHeader() {
	pad(16);
	beginOffset = SDL_RWtell(fp);
	serialize(header);
}

void CBinaryArchiveReader::markInPlaceOffset(size_t offset) {
	inPlaceOffset = beginOffset + offset;
}

void CBinaryArchiveReader::finish() {
	SDL_assert_release(header.unk3 == inPlaceRead);
	if (header.unk3 != inPlaceRead) {
		SDL_Log("Missed inPlace by %i", header.unk3 - inPlaceRead);
		std::vector<uint8_t> extra(header.unk3 - inPlaceRead);
		memBlockInPlace(extra.data(), 1, extra.size());
		for (auto it : extra)
			SDL_assert_release(it == 0);
	}

	pad(4);
	SDL_assert_release(header.unk5 == SDL_RWtell(fp) - beginOffset);

	//Read You know
	head6.resize(header.unk6);
	for (auto &it : head6)
		serialize(it);

	head8.resize(header.unk8);
	for (auto &it : head8)
		serialize(it);

	SDL_assert_release(header.unk1 == SDL_RWtell(fp) - beginOffset);
}

CBinaryArchiveWriter::CBinaryArchiveWriter(SDL_RWops* _fp) {
	fp = _fp;
	bigEndian = settings.bigEndian;
}

CBinaryArchiveWriter::~CBinaryArchiveWriter() {
}

bool CBinaryArchiveWriter::isReading() const {
	return false;
}

void CBinaryArchiveWriter::memBlock(void* ptr, size_t objSize, size_t objCount) {
	size_t ret = SDL_RWwrite(fp, ptr, objSize, objCount);
	SDL_assert_release(ret == objCount);
	offset = SDL_RWtell(fp);
}

void CBinaryArchiveWriter::memBlockInPlace(void* ptr, size_t objSize, size_t objCount) {
	if (numPausedInPlace) {
		memBlock(ptr, objSize, objCount);
	} else {
		inPlaceData.insert(inPlaceData.end(), (uint8_t*)ptr, (uint8_t*)ptr + (objSize * objCount));
		inPlaceRead += objSize * objCount;
	}
}

void CBinaryArchiveWriter::markHeader() {
	pad(16);
	headerOffset = SDL_RWtell(fp);
	serialize(header);
}

void CBinaryArchiveWriter::markInPlaceOffset(size_t offset) {
}

void CBinaryArchiveWriter::finish() {
	if (headerOffset >= 0) {
		header.unk5 = SDL_RWtell(fp) - beginOffset;

		for (auto it : head6)
			serialize(it);

		for (auto it : head8)
			serialize(it);

		//Calculate End Padding
		size_t padding = 16;
		size_t size = SDL_RWtell(fp) - beginOffset;
		size_t seek = (padding - (size % padding)) % padding;

		header.unk1 = SDL_RWtell(fp) - beginOffset;
		header.unk2 = header.unk1 + seek;
		header.unk3 = inPlaceData.size();
		//Unk4 is calculated
		//Unk6 is calculated
		header.unk7 = header.unk6 * 0x10 + header.unk5;
		//Unk8 is calculated
		//Unk9 is calculated

		//Write Padding
		char temp[16] = { 0 };
		SDL_RWwrite(fp, temp, 1, seek);

		//Write Inplace Data
		SDL_RWwrite(fp, inPlaceData.data(), 1, inPlaceData.size());

		//Write back correct Header
		SDL_RWseek(fp, headerOffset, RW_SEEK_SET);
		serialize(header);
	}
}

bool operator==(const IBinaryArchive::Header& lhs, const IBinaryArchive::Header& rhs) {
	return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}
