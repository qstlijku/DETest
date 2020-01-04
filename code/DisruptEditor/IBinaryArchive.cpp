#include "IBinaryArchive.h"
#include "FileHandler.h"

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

void IBinaryArchive::serialize(int8_t& value) {
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
	uint32_t len = (uint32_t)value.size() + 1;
	serialize(len);
	value.resize(len, '\0');
	memBlock(value.data(), 1, len);
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
	fp.serialize(inPlaceOffset);
	fp.serialize(unk2);
	fp.serialize(unk3);
	fp.serialize(unk4);
	fp.serialize(unk5);
	fp.serialize(unk6);
	fp.serialize(unk7);
	fp.serialize(unk8);
	fp.serialize(unk9);

	SDL_assert_release(inPlaceOffset == unk2 || inPlaceOffset == unk5);

	SDL_assert_release(unk5 == unk7);//Size of header + non-inplace serialized data

	SDL_assert_release(unk4 == 0);
	SDL_assert_release(unk6 == 0);

	SDL_assert_release(unk9 == 0);
}

CBinaryArchiveReader::CBinaryArchiveReader(SDL_RWops* _fp) {
	fp = _fp;
}

bool CBinaryArchiveReader::isReading() const {
	return true;
}

void CBinaryArchiveReader::pad(size_t padding) {
	size_t seek = getPadSize(padding);

	Vector<uint8_t> data(seek);
	memBlock(data.data(), 1, seek);
	for (Sint64 i = 0; i < seek; ++i)
		SDL_assert(data[i] == 0);
	offset = SDL_RWtell(fp);
}

void CBinaryArchiveReader::memBlock(void* ptr, size_t objSize, size_t objCount) {
	size_t ret = SDL_RWread(fp, ptr, objSize, objCount);
	SDL_assert_release(ret == objCount);
	offset = SDL_RWtell(fp);
}

void CBinaryArchiveReader::memBlockInPlace(void* ptr, size_t objSize, size_t objCount) {
	SDL_assert_release(inPlaceOffset >= 0);
	Sint64 curOffset = SDL_RWtell(fp);
	SDL_RWseek(fp, inPlaceOffset, RW_SEEK_SET);
	SDL_RWread(fp, ptr, objSize, objCount);
	inPlaceOffset += objSize * objCount;
	SDL_RWseek(fp, curOffset, RW_SEEK_SET);
}

void CBinaryArchiveReader::markHeader() {
	serialize(header);
	inPlaceOffset = header.inPlaceOffset + 0x2f & 0xfffffff0;
}

CBinaryArchiveWriter::CBinaryArchiveWriter(SDL_RWops* _fp) {
	fp = _fp;
}

CBinaryArchiveWriter::~CBinaryArchiveWriter() {
}

bool CBinaryArchiveWriter::isReading() const {
	return false;
}

void CBinaryArchiveWriter::pad(size_t padding) {
	size_t size = getPadSize(padding);
	char temp[64] = { 0 };
	memBlock(temp, 1, size);
}

void CBinaryArchiveWriter::memBlock(void* ptr, size_t objSize, size_t objCount) {
	size_t ret = SDL_RWwrite(fp, ptr, objSize, objCount);
	SDL_assert_release(ret == objCount);
	offset = SDL_RWtell(fp);

	if (headerOffset > 0)
		dataSize += objSize + objCount;
}

void CBinaryArchiveWriter::memBlockInPlace(void* ptr, size_t objSize, size_t objCount) {
	inPlaceData.insert(inPlaceData.end(), (uint8_t*)ptr, (uint8_t*)ptr + (objSize * objCount));
}

void CBinaryArchiveWriter::markHeader() {
	headerOffset = SDL_RWtell(fp);
	IBinaryArchive::Header header;
	serialize(header);
}

void CBinaryArchiveWriter::finish() {
	if (headerOffset > 0) {
		//Write Inplace Data
		pad(16);
		SDL_RWwrite(fp, inPlaceData.data(), 1, inPlaceData.size());

		//TODO: Write back correct Header
		IBinaryArchive::Header header;
		header.inPlaceOffset = dataSize;
		header.unk5 = header.unk7 = dataSize;

		SDL_RWseek(fp, headerOffset, RW_SEEK_SET);
		serialize(header);
	}
}
